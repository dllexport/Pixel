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

void VulkanGroupExecutor::AddRenderGroup(IntrusivePtr<RenderGroup> group)
{
    auto vrp = (VulkanRenderGroup *)group.get();
    this->renderGroups[group->Name()] = vrp;
}

void VulkanGroupExecutor::BindResource(std::string name, std::vector<IntrusivePtr<ResourceHandle>> resources)
{
    this->sharedResources[name] = resources;
}

void VulkanGroupExecutor::Reset()
{
    releaseFences();
    releaseSharedResources();
    releaseRenderCommandBuffers();

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

void VulkanGroupExecutor::releaseFences()
{
    for (auto &fence : queueCompleteFences)
    {
        vkDestroyFence(context->GetVkDevice(), fence, nullptr);
    }
}

void VulkanGroupExecutor::releaseSharedResources()
{
    sharedResources.clear();
}

void VulkanGroupExecutor::prepareSharedResources()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    for (auto &[groupName, renderGroup] : renderGroups)
    {
        for (auto sharedKey : renderGroup->GetGraph()->GetSharedResourceKeys())
        {
            // skip if resource is already allocated
            if (sharedResources[sharedKey].size() == vulkanSC->GetTextures().size())
                continue;
                
            auto &resourceNode = renderGroup->GetGraph()->GetNodeMap().at(sharedKey);

            for (int i = 0; i < vulkanSC->GetTextures().size(); i++)
            {
                switch (resourceNode->type)
                {
                case GraphNode::ATTACHMENT:
                {
                    auto attachmentNode = (AttachmentGraphNode *)resourceNode.get();

                    if (attachmentNode->swapChain)
                        sharedResources[sharedKey].push_back(vulkanSC->GetTextures()[i]);
                    else
                    {
                        auto attachmentTexture = renderGroup->CreateAttachmentResource(vulkanSC, attachmentNode);
                        sharedResources[sharedKey].push_back(attachmentTexture);
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }

        for (auto [key, resources] : sharedResources)
        {
            renderGroup->ImportResource(key, resources);
        }
    }
}

void VulkanGroupExecutor::Prepare()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());
    auto swapChainImageSize = vulkanSC->GetTextures().size();

    prepareFences();
    prepareRenderCommandBuffers();
    prepareSharedResources();

    for (auto &[name, rg] : renderGroups)
    {
        rg->Prepare(vulkanSC);
    }

    prepareGlobalSynchronization();
    prepareRenderGroupSynchronization();
}

std::vector<VkCommandBuffer> VulkanGroupExecutor::createCommandBuffer(uint32_t size)
{
    std::vector<VkCommandBuffer> buffers;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = size;
    buffers.resize(size);
    auto result = vkAllocateCommandBuffers(context->GetVkDevice(), &commandBufferAllocateInfo, buffers.data());
    return buffers;
}

void VulkanGroupExecutor::prepareRenderCommandBuffers()
{
    if (commandPool == VK_NULL_HANDLE)
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = context->GetQueue(VK_QUEUE_GRAPHICS_BIT).familyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        auto result = vkCreateCommandPool(context->GetVkDevice(), &cmdPoolInfo, nullptr, &commandPool);
    }

    auto vulkanSC = static_cast<VulkanSwapChain *>(swapChain.get());
    auto scTextureSize = vulkanSC->GetTextures().size();
    auto buffers = createCommandBuffer(scTextureSize);
    renderCommandBuffers.insert(std::end(renderCommandBuffers), std::begin(buffers), std::end(buffers));
}

void VulkanGroupExecutor::releaseRenderCommandBuffers()
{
    for (auto &commandBuffer : renderCommandBuffers)
    {
        vkResetCommandBuffer(commandBuffer, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }
    renderCommandBuffers.clear();

    renderGroupSyncMap.clear();
    globalSynCommands = {};

    vkDestroyCommandPool(context->GetVkDevice(), commandPool, nullptr);
    commandPool = VK_NULL_HANDLE;
}

void VulkanGroupExecutor::prepareGlobalSynchronization()
{
    globalSynCommands.afterGroupExec = createCommandBuffer(swapChain->ImageSize());
    globalSynCommands.beforeGroupExec = createCommandBuffer(swapChain->ImageSize());

    std::string swapChainKey;
    // search swapchain attachment
    for (auto &[key, resources] : sharedResources)
    {
        if (resources[0]->type != ResourceHandle::TEXTURE)
            continue;
        if (static_cast<VulkanTexture *>(resources[0].get())->IsSwapChain())
            swapChainKey = key;
    }

    assert(!swapChainKey.empty());

    for (int idx = 0; idx < swapChain->ImageSize(); idx++)
    {
        auto presentTexture = static_cast<VulkanTexture *>(sharedResources[swapChainKey][idx].get());

        // for swapchain, build transit layout from present to generel before execute group
        VkCommandBufferBeginInfo cmdBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

        {
            auto &beforeGroupExecCommand = globalSynCommands.beforeGroupExec[idx];
            vkBeginCommandBuffer(beforeGroupExecCommand, &cmdBufferBeginInfo);
            VkImageMemoryBarrier imageMemoryBarrier = {};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageMemoryBarrier.image = presentTexture->GetImage();
            imageMemoryBarrier.subresourceRange = presentTexture->GetImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
            vkCmdPipelineBarrier(beforeGroupExecCommand, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            vkEndCommandBuffer(beforeGroupExecCommand);
        }

        {
            auto &afterGroupExecCommand = globalSynCommands.afterGroupExec[idx];
            vkBeginCommandBuffer(afterGroupExecCommand, &cmdBufferBeginInfo);
            VkImageMemoryBarrier imageMemoryBarrier = {};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            imageMemoryBarrier.image = presentTexture->GetImage();
            imageMemoryBarrier.subresourceRange = presentTexture->GetImageSubResourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
            vkCmdPipelineBarrier(afterGroupExecCommand, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            vkEndCommandBuffer(afterGroupExecCommand);
        }
    }
}

void VulkanGroupExecutor::prepareRenderGroupSynchronization()
{
    auto swapChainSize = swapChain->ImageSize();

    std::vector<std::reference_wrapper<IntrusivePtr<VulkanRenderGroup>>> groups;

    // aggregate command buffers
    for (auto &[_, rg] : renderGroups)
    {
        groups.insert(groups.begin(), rg);
    }

    for (int i = 0; i < groups.size(); i++)
    {
        auto &rg = groups[i].get();

        renderGroupSyncMap[rg->Name()].beforeGroupExec = createCommandBuffer(swapChainSize);
        renderGroupSyncMap[rg->Name()].afterGroupExec = createCommandBuffer(swapChainSize);

        auto &cbs = renderGroupSyncMap[rg->Name()];

        for (int idx = 0; idx < swapChainSize; idx++)
        {
            // for swapchain, build transit layout from present to generel before execute group
            auto &commandBuffer = cbs.afterGroupExec[idx];
            VkCommandBufferBeginInfo cmdBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
            vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

            VkMemoryBarrier memoryBarrier = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

            vkCmdPipelineBarrier(
                renderGroupSyncMap[rg->Name()].afterGroupExec[idx],
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_DEPENDENCY_BY_REGION_BIT,
                1,
                &memoryBarrier, 0, nullptr, 0, nullptr);

            vkEndCommandBuffer(commandBuffer);
        }
    }
}

bool VulkanGroupExecutor::Execute()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<std::reference_wrapper<IntrusivePtr<VulkanRenderGroup>>> groups;

    // aggregate command buffers
    for (auto &[_, rg] : renderGroups)
    {
        groups.insert(groups.begin(), rg);
    }

    commandBuffers.push_back(globalSynCommands.beforeGroupExec[currentImage]);

    for (int i = 0; i < groups.size(); i++)
    {
        auto &rg = groups[i];
        auto cbs = rg.get()->GetCommandBuffer(currentImage);
        commandBuffers.insert(commandBuffers.end(), cbs.begin(), cbs.end());
        commandBuffers.push_back(renderGroupSyncMap[rg.get()->Name()].afterGroupExec[currentImage]);
    }

    commandBuffers.push_back(globalSynCommands.afterGroupExec[currentImage]);

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
