#include <RHI/VulkanRuntime/RenderGroupExecutor.h>
#include <RHI/VulkanRuntime/ResourceBindingState.h>

#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/SwapChain.h>
#include <RHI/VulkanRuntime/Sampler.h>
#include <RHI/VulkanRuntime/Image.h>

#include <RHI/ConstantBuffer.h>

#include <FrameGraph/Graph.h>

#include <spdlog/spdlog.h>

VulkanGroupExecutor::VulkanGroupExecutor(IntrusivePtr<Context> context) : context(context)
{
}

VulkanGroupExecutor::~VulkanGroupExecutor()
{
    vkDeviceWaitIdle(context->GetVkDevice());
    Reset();
}

void VulkanGroupExecutor::AddBindingState(IntrusivePtr<ResourceBindingState> state)
{
    auto vgp = static_cast<VulkanGraphicsPipeline *>(state->GetPipeline().get());

    for (auto &[name, rg] : renderGroups)
    {
        if (rg->GetGraph()->graphNodesMap.count(vgp->GetPipelineName()))
        {
            rg->AddBindingState(state);
            break;
        }
    }
}

void VulkanGroupExecutor::Reset()
{
    for (auto &fence : queueCompleteFences)
    {
        vkDestroyFence(context->GetVkDevice(), fence, nullptr);
    }

    queueCompleteFences.clear();

    currentFrame = 0;
    currentImage = 0;

    for (auto &[name, rg] : renderGroups)
    {
        rg->Reset();
    }
}

void VulkanGroupExecutor::prepareFences()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    queueCompleteFences.resize(vulkanSC->GetTextures().size());
    for (auto &fence : queueCompleteFences)
    {
        vkCreateFence(context->GetVkDevice(), &fenceCreateInfo, nullptr, &fence);
    }
}

void VulkanGroupExecutor::Prepare()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());
    auto swapChainImageSize = vulkanSC->GetTextures().size();

    prepareFences();

    for (auto &[name, rg] : renderGroups)
    {
        rg->Prepare(vulkanSC);
    }
}

bool VulkanGroupExecutor::Execute()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    std::vector<VkCommandBuffer> commandBuffers;

    // aggregate command buffers
    for (auto &[_, rg] : renderGroups)
    {
        auto cbs = rg->GetCommandBuffer(currentImage);
        commandBuffers.insert(commandBuffers.end(), cbs.begin(), cbs.end());
    }

    VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {vulkanSC->GetImageAvailableSemaphores()[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    VkSemaphore signalSemaphores[] = {vulkanSC->GetRenderFinishedSemaphores()[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context->GetQueue(VK_QUEUE_GRAPHICS_BIT).queue, 1, &submitInfo, queueCompleteFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    bool presentResult = swapChain->Present(currentImage, currentFrame);
    if (!presentResult)
    {
        return false;
    }

    currentFrame = (currentFrame + 1) % vulkanSC->GetTextures().size();
    return true;
}

uint32_t VulkanGroupExecutor::CurrentImage()
{
    return currentImage;
}

uint32_t VulkanGroupExecutor::Acquire()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    vkWaitForFences(context->GetVkDevice(), 1, &queueCompleteFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->GetVkDevice(), 1, &queueCompleteFences[currentFrame]);

    currentImage = vulkanSC->Acquire(currentFrame);
    if (currentImage == -1)
    {
        spdlog::info("acquire failed");
        return -1;
    }

    return currentImage;
}

void VulkanGroupExecutor::Update()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    // aggregate command buffers
    for (auto [_, rg] : this->renderGroups)
    {
        rg->Update(currentImage, vulkanSC);
    }
}

void VulkanGroupExecutor::WaitIdle()
{
    vkDeviceWaitIdle(context->GetVkDevice());
}
