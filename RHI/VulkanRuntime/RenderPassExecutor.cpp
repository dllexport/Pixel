#include <RHI/VulkanRuntime/RenderPassExecutor.h>
#include <RHI/VulkanRuntime/ResourceBindingState.h>

#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/SwapChain.h>

#include <FrameGraph/Graph.h>

#include <spdlog/spdlog.h>

VulkanRenderPassExecutor::VulkanRenderPassExecutor(IntrusivePtr<Context> context) : context(context)
{
    prepareCommandPool();
}

VulkanRenderPassExecutor::~VulkanRenderPassExecutor()
{
    vkDeviceWaitIdle(context->GetVkDevice());

    Reset();

    vkDestroyCommandPool(context->GetVkDevice(), graphicCommandPool, nullptr);
    vkDestroyCommandPool(context->GetVkDevice(), computeCommandPool, nullptr);
}

void VulkanRenderPassExecutor::Reset()
{
    attachmentImages.clear();
    sharedImages.clear();

    for (auto &[_, frameBuffer] : frameBuffers)
    {
        for (auto &fb : frameBuffer)
        {
            vkDestroyFramebuffer(context->GetVkDevice(), fb, nullptr);
        }

        frameBuffer.clear();
    }

    for (auto &[_, graphicCommandBuffer] : graphicCommandBuffers)
    {
        graphicCommandBuffer.clear();
    }

    for (auto &fence : queueCompleteFences)
    {
        vkDestroyFence(context->GetVkDevice(), fence, nullptr);
    }

    queueCompleteFences.clear();
}

void VulkanRenderPassExecutor::prepareFences()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Create in signaled state so we don't wait on first render of each command buffer
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    queueCompleteFences.resize(vulkanSC->GetTextures().size());
    for (auto &fence : queueCompleteFences)
    {
        vkCreateFence(context->GetVkDevice(), &fenceCreateInfo, nullptr, &fence);
    }
}

void VulkanRenderPassExecutor::prepareCommandPool()
{
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = context->GetQueue(VK_QUEUE_GRAPHICS_BIT).familyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        auto result = vkCreateCommandPool(context->GetVkDevice(), &cmdPoolInfo, nullptr, &graphicCommandPool);
    }

    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = context->GetQueue(VK_QUEUE_COMPUTE_BIT).familyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        auto result = vkCreateCommandPool(context->GetVkDevice(), &cmdPoolInfo, nullptr, &computeCommandPool);
    }
}

void VulkanRenderPassExecutor::prepareCommandBuffer()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    for (auto &renderPass : renderPasses)
    {
        std::vector<VkCommandBuffer> cbs(1);
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = graphicCommandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = 1;
        auto result = vkAllocateCommandBuffers(context->GetVkDevice(), &commandBufferAllocateInfo, cbs.data());
        graphicCommandBuffers[renderPass].push_back(cbs[0]);
    }
}

void VulkanRenderPassExecutor::buildCommandBuffer(uint32_t imageIndex)
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    for (auto &renderPass : renderPasses)
    {
        auto vulkanRP = static_cast<VulkanRenderPass *>(renderPass.get());
        auto &commandBuffer = graphicCommandBuffers[vulkanRP][imageIndex];

        VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        std::vector<VkClearValue> clearValues(vulkanRP->attachmentDescriptions.size());
        clearValues[0].color = {{0.0f, 0.0f, 0.2f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vulkanRP->GetRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = vulkanSC->extent;
        renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();
        renderPassBeginInfo.framebuffer = frameBuffers[vulkanRP][imageIndex];

        vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = vulkanSC->extent.width;
        viewport.height = vulkanSC->extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent = vulkanSC->extent;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        std::unordered_map<std::string, IntrusivePtr<VulkanPipeline>> subPassName2Pipeline;

        for (auto [pipeline, resourceBinding] : resourceBindingStates)
        {
            auto vulkanPL = static_cast<VulkanPipeline *>(pipeline.get());
            subPassName2Pipeline[vulkanPL->GetSubPassName()] = vulkanPL;
        }

        auto graph = renderPass->GetGraph();
        auto topoResult = graph->Topo();
        for (auto &[k, rps] : topoResult.levelsRenderPassOnly)
        {
            for (auto &node : rps)
            {
                auto vulkanPL = subPassName2Pipeline[node->name];
                auto vrbs = resourceBindingStates[vulkanPL];
                for (auto vrb : vrbs)
                {
                    auto vulkanRBS = static_cast<VulkanResourceBindingState *>(vrb.get());
                    auto pipelineLayout = vulkanPL->GetPipelineLayout();
                    auto descriptorSets = vulkanRBS->GetDescriptorSets();

                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPL->GetPipeline());
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

                    const VkDeviceSize offsets[1] = {0};
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vulkanRBS->GetVertexBuffer()->GetBuffer(), offsets);
                    vkCmdBindIndexBuffer(commandBuffer, vulkanRBS->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

                    vkCmdDrawIndexed(commandBuffer, 3, 1, 0, 0, 1);
                }
            }
        }

        vkCmdEndRenderPass(commandBuffer);

        vkEndCommandBuffer(commandBuffer);
    }
}

uint32_t DeferAttachmentUsage(IntrusivePtr<AttachmentGraphNode> attachmentNode)
{
    uint32_t usage = 0;

    if (attachmentNode->depthStencil)
    {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (attachmentNode->input)
    {
        usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (attachmentNode->color)
    {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    return usage;
}

void VulkanRenderPassExecutor::Prepare()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());
    auto swapChainImageSize = vulkanSC->GetTextures().size();

    prepareFences();

    attachmentImages.resize(swapChainImageSize);

    for (int i = 0; i < swapChainImageSize; i++)
    {
        std::vector<VkImageView> attachmentViews;

        for (auto &renderPass : renderPasses)
        {
            auto vulkanRP = static_cast<VulkanRenderPass *>(renderPass.get());

            for (auto attachment : vulkanRP->attachmentDescriptions)
            {
                if (attachment->swapChain)
                {
                    sharedImages[attachment->name].emplace_back(vulkanSC->GetTextures()[i], vulkanSC->GetTextureViews()[i]);
                    attachmentViews.push_back(vulkanSC->GetTextureViews()[i]->GetImageView());
                    continue;
                }

                // if shared attachment is already created, take it's view cache
                if (attachment->shared && sharedImages.count(attachment->name))
                {
                    attachmentViews.push_back(sharedImages[attachment->name][0].textureView->GetImageView());
                    continue;
                }

                IntrusivePtr<VulkanTexture> texture = new VulkanTexture(context);
                auto textureExtent = Texture::Extent{
                    .width = vulkanSC->extent.width,
                    .height = vulkanSC->extent.height,
                    .depth = 1};

                auto textureConfig = Texture::Configuration{
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = 1,
                    .tiling = Texture::Tiling::IMAGE_TILING_OPTIMAL};

                auto textureUsage = DeferAttachmentUsage(attachment);

                texture->Allocate(attachment->format, textureUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureExtent, textureConfig);

                VkImageAspectFlags imageAspect = {};

                if (textureUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    imageAspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
                }
                if (textureUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                {
                    imageAspect |= VK_IMAGE_ASPECT_COLOR_BIT;
                }
                VkImageViewCreateInfo imageView = {};
                imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
                imageView.format = GeneralFormatToVkFormat(attachment->format);
                imageView.subresourceRange = {};
                imageView.subresourceRange.aspectMask = imageAspect;
                imageView.subresourceRange.baseMipLevel = 0;
                imageView.subresourceRange.levelCount = 1;
                imageView.subresourceRange.baseArrayLayer = 0;
                imageView.subresourceRange.layerCount = 1;
                imageView.image = texture->GetImage();

                auto textureView = texture->CreateTextureView(imageView);

                attachmentImages.emplace_back(texture, textureView);

                if (attachment->shared && !sharedImages.count(attachment->name))
                {
                    sharedImages[attachment->name].emplace_back(texture, textureView);
                }

                attachmentViews.push_back(textureView->GetImageView());
            }

            VkFramebufferCreateInfo frameBufferCreateInfo = {};
            frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frameBufferCreateInfo.renderPass = vulkanRP->GetRenderPass();
            frameBufferCreateInfo.attachmentCount = (uint32_t)attachmentViews.size();
            frameBufferCreateInfo.pAttachments = attachmentViews.data();
            frameBufferCreateInfo.width = vulkanSC->extent.width;
            frameBufferCreateInfo.height = vulkanSC->extent.height;
            frameBufferCreateInfo.layers = 1;

            VkFramebuffer frameBuffer;
            auto result = vkCreateFramebuffer(context->GetVkDevice(), &frameBufferCreateInfo, nullptr, &frameBuffer);
            frameBuffers[vulkanRP].push_back(frameBuffer);
        }

        prepareCommandBuffer();

        buildCommandBuffer(i);
    }
}

bool VulkanRenderPassExecutor::Execute()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    vkWaitForFences(context->GetVkDevice(), 1, &queueCompleteFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->GetVkDevice(), 1, &queueCompleteFences[currentFrame]);

    auto imageIndex = vulkanSC->Acquire(currentFrame);
    if (imageIndex == -1)
    {
        spdlog::info("acquire failed");
        return false;
    }

    std::vector<VkCommandBuffer> commandBuffers;
    for (auto cb : updateCommandBuffers)
    {
        commandBuffers.push_back(cb);
    }

    // aggregate command buffers
    for (auto [_, cbs] : graphicCommandBuffers)
    {
        commandBuffers.push_back(cbs[currentFrame]);
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

    bool presentResult = swapChain->Present(imageIndex, currentFrame);
    if (!presentResult)
    {
        return false;
    }

    currentFrame = (currentFrame + 1) % vulkanSC->GetTextures().size();
}

void VulkanRenderPassExecutor::Update()
{
}

void VulkanRenderPassExecutor::WaitIdle()
{
    vkDeviceWaitIdle(context->GetVkDevice());
}
