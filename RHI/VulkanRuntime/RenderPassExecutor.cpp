#include <RHI/VulkanRuntime/RenderPassExecutor.h>
#include <RHI/VulkanRuntime/ResourceBindingState.h>

#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/SwapChain.h>
#include <RHI/VulkanRuntime/Sampler.h>
#include <RHI/VulkanRuntime/Image.h>

#include <RHI/ConstantBuffer.h>

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
    for (auto [_, resource] : renderPassResourceMap)
    {
        for (auto resetEntry : resource.resetEntries)
        {
            resource.attachmentImages.erase(resetEntry);
        }

        for (auto &fb : resource.frameBuffers)
        {
            vkDestroyFramebuffer(context->GetVkDevice(), fb, nullptr);
        }

        resource.frameBuffers.clear();

        for (auto &graphicCommandBuffer : resource.graphicCommandBuffers)
        {
            vkResetCommandBuffer(graphicCommandBuffer, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }
        resource.graphicCommandBuffers.clear();
    }

    renderPassResourceMap.clear();

    for (auto &fence : queueCompleteFences)
    {
        vkDestroyFence(context->GetVkDevice(), fence, nullptr);
    }

    queueCompleteFences.clear();

    currentFrame = 0;
    currentImage = 0;
}

void VulkanRenderPassExecutor::ResetSwapChainImages()
{
    // TODO
}

void VulkanRenderPassExecutor::prepareFences()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
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

void VulkanRenderPassExecutor::buildCommandBuffer(uint32_t imageIndex)
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    for (auto &renderPass : renderPasses)
    {
        auto vulkanRP = static_cast<VulkanRenderPass *>(renderPass.get());
        auto &renderPassResource = renderPassResourceMap[renderPass];
        auto &commandBuffer = renderPassResource.graphicCommandBuffers[imageIndex];

        VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        std::vector<VkClearValue> clearValues(vulkanRP->attachmentNodes.size());
        for (int i = 0; i < vulkanRP->attachmentNodes.size(); i++)
        {
            auto attachmentNode = vulkanRP->attachmentNodes[i];
            if (attachmentNode->color)
                clearValues[i].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

            if (attachmentNode->depthStencil)
                clearValues[i].depthStencil = {1.0f, 0};
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vulkanRP->GetRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = vulkanSC->extent;
        renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();
        renderPassBeginInfo.framebuffer = renderPassResource.frameBuffers[imageIndex];

        vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = vulkanSC->extent.width;
        viewport.height = vulkanSC->extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        std::unordered_map<std::string, IntrusivePtr<VulkanPipeline>> subPassName2Pipeline;

        for (auto [pipeline, resourceBinding] : resourceBindingStates)
        {
            auto vulkanPL = static_cast<VulkanPipeline *>(pipeline.get());
            subPassName2Pipeline[vulkanPL->GetSubPassName()] = vulkanPL;
        }

        auto graph = renderPass->GetGraph();
        auto topoResult = graph->Topo();
        bool firstPass = true;
        for (auto &[level, rps] : topoResult.levelsRenderPassOnly)
        {
            for (auto &node : rps)
            {
                auto vulkanPL = subPassName2Pipeline[node->name];
                if (!resourceBindingStates.count(vulkanPL))
                {
                    continue;
                }
                auto vrbs = resourceBindingStates[vulkanPL];
                // for each drawable of pipeline
                for (int rpIndex = 0; rpIndex < vrbs.size(); rpIndex++)
                {
                    auto &vrb = vrbs[rpIndex];
                    auto vulkanRBS = static_cast<VulkanResourceBindingState *>(vrb.get());
                    auto &pipelineLayout = vulkanPL->GetPipelineLayout();
                    auto &descriptorSets = vulkanRBS->GetDescriptorSets(imageIndex);
                    auto constantBuffer = static_cast<ConstantBuffer *>(vulkanRBS->GetConstantBuffer().get());

                    if (vrb->GetDrawOps().empty() && (!vulkanRBS->GetVertexBuffers() || !vulkanRBS->GetIndexBuffers()))
                    {
                        continue;
                    }

                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPL->GetPipeline());

                    if (constantBuffer)
                    {
                        vkCmdPushConstants(commandBuffer, pipelineLayout->GetLayout(),
                                           VK_SHADER_STAGE_VERTEX_BIT,
                                           0, constantBuffer->Size(), constantBuffer->Map());
                    }

                    if (!descriptorSets.empty())
                    {
                        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
                    }

                    const VkDeviceSize offsets[1] = {0};
                    auto vertexBuffer = vulkanRBS->GetVertexBuffer(imageIndex);
                    if (vertexBuffer)
                    {
                        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer->GetBuffer(), offsets);
                    }
                    auto indexBuffer = vulkanRBS->GetIndexBuffer(imageIndex);
                    if (indexBuffer)
                    {
                        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, vulkanRBS->GetIndexType());
                    }

                    // default scissor
                    VkRect2D scissor = {};
                    scissor.extent = vulkanSC->extent;
                    scissor.offset.x = 0;
                    scissor.offset.y = 0;

                    for (auto drawOP : vulkanRBS->GetDrawOps())
                    {
                        if (drawOP.scissorExtent.x && drawOP.scissorExtent.y)
                        {
                            scissor.offset = {drawOP.scissorOffset.x, drawOP.scissorOffset.y};
                            scissor.extent = {drawOP.scissorExtent.x, drawOP.scissorExtent.y};
                        }
                        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
                        if (drawOP.indexCount)
                            vkCmdDrawIndexed(commandBuffer, drawOP.indexCount, drawOP.instanceCount, drawOP.firstIndex, drawOP.vertexOffset, drawOP.firstInstance);
                        else if (drawOP.vertexCount)
                            vkCmdDraw(commandBuffer, drawOP.vertexCount, drawOP.instanceCount, 0, 0);
                    }
                }
            }

            if (level < topoResult.maxLevelRenderPassOnly)
            {
                vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
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
    if (attachmentNode->inputSubPassNames.size() != 0)
    {
        usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (attachmentNode->color)
    {
        usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    return usage;
}

void VulkanRenderPassExecutor::prepareCommandBuffer(IntrusivePtr<RenderPass> &renderPass)
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());
    auto scTextureSize = vulkanSC->GetTextures().size();
    auto &graphicCommandBuffers = renderPassResourceMap[renderPass].graphicCommandBuffers;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = graphicCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = uint32_t(scTextureSize);
    graphicCommandBuffers.resize(scTextureSize);
    auto result = vkAllocateCommandBuffers(context->GetVkDevice(), &commandBufferAllocateInfo, graphicCommandBuffers.data());
}

void VulkanRenderPassExecutor::prepareFrameBuffer(IntrusivePtr<RenderPass> &renderPass)
{
    auto vulkanRP = static_cast<VulkanRenderPass *>(renderPass.get());
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    auto &renderPassResource = this->renderPassResourceMap[renderPass];

    auto &attachmentImages = renderPassResource.attachmentImages;

    renderPassResource.frameBuffers.resize(vulkanSC->GetTextures().size());

    for (int i = 0; i < vulkanSC->GetTextures().size(); i++)
    {
        std::vector<VkImageView> attachmentViews;

        for (auto attachment : vulkanRP->attachmentNodes)
        {
            // TODO: skip if image already exist
            // if attachment is swapchain, use reference instead of creating one
            if (attachment->swapChain)
            {
                attachmentImages[attachment->name].push_back({vulkanSC->GetTextures()[i], vulkanSC->GetTextureViews()[i], nullptr});
                attachmentViews.push_back(vulkanSC->GetTextureViews()[i]->GetImageView());
                renderPassResource.resetEntries.insert(attachment->name);
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

            spdlog::info("frame {} creating {}", i, attachment->name);
            texture->Allocate(attachment->format, textureUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureExtent, textureConfig);

            VkImageAspectFlags imageAspect = {};

            if (textureUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                imageAspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
                // need to recreate depth texture when swapchain change
                renderPassResource.resetEntries.insert(attachment->name);
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

            IntrusivePtr<VulkanSampler> sampler;
            if (attachment->inputSubPassNames.size() != 0)
            {
                sampler = new VulkanSampler(context, texture);
                sampler->Allocate({});
            }

            attachmentImages[attachment->name].push_back({texture, textureView, sampler});
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
        renderPassResource.frameBuffers[i] = frameBuffer;
    }
}

void VulkanRenderPassExecutor::prepareRenderPassResource(IntrusivePtr<RenderPass> &renderPass)
{
    prepareCommandBuffer(renderPass);
    prepareFrameBuffer(renderPass);
}

void VulkanRenderPassExecutor::Prepare()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());
    auto swapChainImageSize = vulkanSC->GetTextures().size();

    prepareFences();

    for (auto renderPass : renderPasses)
    {
        prepareRenderPassResource(renderPass);
    }

    // make sure all descriptor set in layout is valid
    // fill dummy or internal data if slot is empty
    resolveDrawStatesDescriptors();
}

void VulkanRenderPassExecutor::resolveDrawStatesDescriptors()
{
    for (auto &[pipeline, drawStates] : resourceBindingStates)
    {
        auto vulkanRP = static_cast<VulkanRenderPass *>(pipeline->GetRenderPass().get());
        auto vulkanPL = static_cast<VulkanPipeline *>(pipeline.get());
        auto pipelineName = vulkanPL->GetSubPassName();
        auto subPassNode = vulkanRP->GetGraphicRenderPassGraphNode(vulkanRP->GetSubPassIndex(pipelineName));

        auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());
        auto swapChainImageSize = vulkanSC->GetTextures().size();

        // check every set binding of pipeline
        // if the resourceName is in attachmentImages, then it's the internal one

        for (auto &drawState : drawStates)
        {
            for (auto &[resourceName, bindingSet] : subPassNode->bindingSets)
            {
                spdlog::info("checking resource {} at {} {}", resourceName, bindingSet.set, bindingSet.binding);

                auto vulkanDrawState = static_cast<VulkanResourceBindingState *>(drawState.get());

                // for each frame, bind internal resource
                for (auto frameIndex = 0; frameIndex < swapChainImageSize; frameIndex++)
                {
                    auto resourceHandleMap = vulkanDrawState->GetResourceHandlesMap(frameIndex);
                    // set empty slot only, no override
                    if (resourceHandleMap[bindingSet.set][bindingSet.binding].empty())
                    {
                        spdlog::info("frame index {} is empty", frameIndex);

                        if (frameIndex == 0 || bindingSet.type == GraphNode::ATTACHMENT)
                        {
                            auto &attachmentImages = renderPassResourceMap[vulkanRP].attachmentImages[resourceName];
                            vulkanDrawState->BindInternal(frameIndex, bindingSet.set, bindingSet.binding, attachmentImages[frameIndex].sampler);
                        }
                        else
                        {
                            vulkanDrawState->Copy(frameIndex, bindingSet.set, bindingSet.binding);
                        }
                    }
                }
            }
        }
    }
}

bool VulkanRenderPassExecutor::Execute()
{
    auto vulkanSC = static_cast<VulkanSwapChain *>(this->swapChain.get());

    std::vector<VkCommandBuffer> commandBuffers;

    // aggregate command buffers
    for (auto [_, cbs] : renderPassResourceMap)
    {
        commandBuffers.push_back(cbs.graphicCommandBuffers[currentImage]);
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

uint32_t VulkanRenderPassExecutor::CurrentImage()
{
    return currentImage;
}

uint32_t VulkanRenderPassExecutor::Acquire()
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

void VulkanRenderPassExecutor::Update()
{
    // aggregate command buffers
    for (auto [_, cbs] : this->renderPassResourceMap)
    {
        vkResetCommandBuffer(cbs.graphicCommandBuffers[currentImage], 0);
    }

    // rebuild the command buffer at lastFrame index
    buildCommandBuffer(currentImage);
}

void VulkanRenderPassExecutor::WaitIdle()
{
    vkDeviceWaitIdle(context->GetVkDevice());
}
