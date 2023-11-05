#include <RHI/VulkanRuntime/RenderGroup.h>

#include <spdlog/spdlog.h>

#include <RHI/ConstantBuffer.h>

#include <RHI/VulkanRuntime/PipelineLayout.h>

#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/ComputePipeline.h>

VulkanRenderGroup::VulkanRenderGroup(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph, IntrusivePtr<VulkanAuxiliaryExecutor> auxiliaryExecutor) : RenderGroup(graph), context(context), auxiliaryExecutor(auxiliaryExecutor)
{
    prepareCommandPool();
}

VulkanRenderGroup::~VulkanRenderGroup()
{
    vkDeviceWaitIdle(context->GetVkDevice());

    Reset();

    vkDestroyCommandPool(context->GetVkDevice(), graphicCommandPool, nullptr);
    vkDestroyCommandPool(context->GetVkDevice(), computeCommandPool, nullptr);
}

void VulkanRenderGroup::Build()
{
    for (auto &[level, passes] : this->graph->Topo().levelsRenderPassOnly)
    {
        for (auto &pass : passes)
        {
            spdlog::info("{} {}", level, pass->GlobalName());
            if (pass->type == GraphNode::GRAPHIC_PASS)
            {
                auto grp = new VulkanGraphicPass(context, graph);
                grp->Build({pass->GlobalName()});
                this->renderPasses[pass->LocalName()] = grp;
            }
            else if (pass->type == GraphNode::COMPUTE_PASS)
            {
                auto grp = new VulkanComputePass(context, graph);
                grp->Build(pass->GlobalName());
                this->computePasses[pass->LocalName()] = grp;
            }
        }
    }
}

void VulkanRenderGroup::Update(uint32_t currentImageIndex, VulkanSwapChain *swapChain)
{
    for (auto [_, cbs] : this->renderPassResourceMap)
    {
        vkResetCommandBuffer(cbs.commandBuffers[currentImageIndex], 0);
    }

    for (auto [_, cbs] : this->computePassResourceMap)
    {
        vkResetCommandBuffer(cbs.commandBuffers[currentImageIndex], 0);
    }

    this->auxiliaryExecutor->Execute();

    // rebuild the command buffer at lastFrame index
    buildCommandBuffer(currentImageIndex, swapChain);
}

void VulkanRenderGroup::Reset()
{
    for (auto &[_, resource] : renderPassResourceMap)
    {
        resource.attachmentImages.clear();

        for (auto &fb : resource.frameBuffers)
        {
            vkDestroyFramebuffer(context->GetVkDevice(), fb, nullptr);
        }

        resource.frameBuffers.clear();

        vkDestroyCommandPool(context->GetVkDevice(), graphicCommandPool, nullptr);
        vkDestroyCommandPool(context->GetVkDevice(), computeCommandPool, nullptr);

        graphicCommandPool = nullptr;
        computeCommandPool = nullptr;

        resource.commandBuffers.clear();
    }

    for (auto &[pipeline, drawStates] : resourceBindingStates)
    {
        for (auto &drawState : drawStates)
        {
            static_cast<VulkanResourceBindingState *>(drawState.get())->GetDescriptorSet()->ClearInternal();
        }
    }

    this->auxiliaryExecutor->Reset();

    sharedResources.clear();
    groupScopeResources.clear();
    prepareCommandPool();
}

void VulkanRenderGroup::AddBindingState(IntrusivePtr<ResourceBindingState> state)
{
    auto vgp = static_cast<VulkanGraphicsPipeline *>(state->GetPipeline().get());
    this->resourceBindingStates[vgp].push_back(static_cast<VulkanResourceBindingState *>(state.get()));
    this->pipelineMap[vgp->pipelineName] = vgp;
}

void VulkanRenderGroup::Prepare(VulkanSwapChain *swapChain)
{
    for (auto &[name, renderPass] : renderPasses)
    {
        prepareCommandBuffer(renderPass, swapChain);
        prepareFrameBuffer(renderPass, swapChain);
    }

    for (auto &[name, computePass] : computePasses)
    {
        // prepareResources(computePass, swapChain);
    }

    // make sure all descriptor set in layout is valid
    // fill dummy or internal data if slot is empty
    resolveDrawStatesDescriptors(swapChain);
}

void VulkanRenderGroup::RegisterPipeline(std::string name, IntrusivePtr<VulkanGraphicsPipeline> pipeline)
{
    pipelineMap[name] = pipeline;
}

void VulkanRenderGroup::ImportResource(std::string name, std::vector<IntrusivePtr<ResourceHandle>> resources)
{
    this->sharedResources[name] = resources;
}

IntrusivePtr<Pipeline> VulkanRenderGroup::CreatePipeline(std::string subPassName, PipelineStates pipelineStates)
{
    auto rp = this->GetRenderPass(subPassName);
    if (!rp)
    {
        spdlog::info("subPass: {} not exist in renderPass {}", subPassName, this->GetGraph()->GetName());
        return nullptr;
    }
    auto pipeline = new VulkanGraphicsPipeline(context, rp, Name(), subPassName, pipelineStates);
    pipeline->groupName = this->Name();
    pipeline->Build();
    return pipeline;
}

IntrusivePtr<Pipeline> VulkanRenderGroup::CreatePipeline(std::string subPassName, ComputePipelineStates pipelineStates)
{
    auto rp = computePasses.at(subPassName);
    if (!rp)
    {
        spdlog::info("subPass: {} not exist in renderPass {}", subPassName, this->GetGraph()->GetName());
        return nullptr;
    }
    auto pipeline = new VulkanComputePipeline(context, rp, Name(), subPassName, pipelineStates);
    pipeline->groupName = this->Name();
    pipeline->Build();
    return pipeline;
}

std::vector<VkCommandBuffer> VulkanRenderGroup::GetCommandBuffer(uint32_t currentImageIndex)
{
    std::vector<VkCommandBuffer> result;
    // TODO, sort by topo
    for (auto [_, cbs] : renderPassResourceMap)
    {
        result.push_back(cbs.commandBuffers[currentImageIndex]);
    }
    return result;
}

IntrusivePtr<VulkanGraphicPass> VulkanRenderGroup::GetRenderPass(std::string name)
{
    return renderPasses[name];
}

void VulkanRenderGroup::prepareCommandPool()
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

void VulkanRenderGroup::buildCommandBuffer(uint32_t imageIndex, VulkanSwapChain *swapchain)
{
    // renderPasses may contain more than 1 subpass
    for (auto &[passName, rp] : renderPasses)
    {
        auto renderPass = static_cast<VulkanGraphicPass *>(rp.get());
        auto &renderPassResource = renderPassResourceMap[renderPass];
        auto &commandBuffer = renderPassResource.commandBuffers[imageIndex];

        VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        std::vector<VkClearValue> clearValues(renderPass->attachmentNodes.size());
        for (int i = 0; i < renderPass->attachmentNodes.size(); i++)
        {
            auto attachmentNode = renderPass->attachmentNodes[i];
            if (attachmentNode->color)
                clearValues[i].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

            if (attachmentNode->depthStencil)
                clearValues[i].depthStencil = {1.0f, 0};
        }

        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass->GetRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent = swapchain->extent;
        renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();
        renderPassBeginInfo.framebuffer = renderPassResource.frameBuffers[imageIndex];

        vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = swapchain->extent.width;
        viewport.height = swapchain->extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        // for each subpass in renderPass
        for (auto &subpass : renderPass->graphicRenderPasses)
        {
            assert(pipelineMap.count(subpass->LocalName()));
            auto pipeline = static_cast<VulkanGraphicsPipeline *>(pipelineMap[passName].get());

            assert(this->resourceBindingStates.count(pipeline));

            auto &drawStates = this->resourceBindingStates[pipeline];

            // if (drawStates.empty())
            // {
            //     vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
            // }

            // for each drawable of pipeline
            for (auto &drawState : drawStates)
            {
                auto &pipelineLayout = pipeline->GetPipelineLayout();
                auto &descriptorSets = drawState->GetDescriptorSets(imageIndex);
                auto constantBuffer = static_cast<ConstantBuffer *>(drawState->GetConstantBuffer().get());

                if (drawState->GetDrawOps().empty() && (!drawState->GetVertexBuffers() || !drawState->GetIndexBuffers()))
                {
                    continue;
                }

                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

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
                auto vertexBuffer = drawState->GetVertexBuffer(imageIndex);
                if (vertexBuffer)
                {
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer->GetBuffer(), offsets);
                }
                auto indexBuffer = drawState->GetIndexBuffer(imageIndex);
                if (indexBuffer)
                {
                    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->GetBuffer(), 0, drawState->GetIndexType());
                }

                // default scissor
                VkRect2D scissor = {};
                scissor.extent = swapchain->extent;
                scissor.offset.x = 0;
                scissor.offset.y = 0;

                for (auto drawOP : drawState->GetDrawOps())
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

        vkCmdEndRenderPass(commandBuffer);

        vkEndCommandBuffer(commandBuffer);
    }

    for (auto &[name, cp] : computePasses)
    {
        auto computePass = static_cast<VulkanComputePass *>(cp.get());
        auto &computePassResource = computePassResourceMap[computePass];
        auto &commandBuffer = computePassResource.commandBuffers[imageIndex];

        VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);
        vkEndCommandBuffer(commandBuffer);
    }
}

uint32_t VulkanRenderGroup::DeferAttachmentUsage(IntrusivePtr<AttachmentGraphNode> attachmentNode)
{
    uint32_t usage = 0;

    if (attachmentNode->depthStencil)
    {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (attachmentNode->inputSubPassNames.size() != 0 || attachmentNode->input)
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

VkImageAspectFlags VulkanRenderGroup::DeferAttachmentAspect(IntrusivePtr<AttachmentGraphNode> attachmentNode)
{
    uint32_t attachmentUsage = DeferAttachmentUsage(attachmentNode);

    VkImageAspectFlags imageAspect = {};

    if (attachmentUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        imageAspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (attachmentUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        imageAspect |= VK_IMAGE_ASPECT_COLOR_BIT;
    }

    return imageAspect;
}

void VulkanRenderGroup::prepareCommandBuffer(IntrusivePtr<VulkanGraphicPass> &renderPass, VulkanSwapChain *swapChain)
{
    auto scTextureSize = swapChain->GetTextures().size();
    {
        auto &commandBuffers = renderPassResourceMap[renderPass].commandBuffers;

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = graphicCommandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = uint32_t(scTextureSize);
        commandBuffers.resize(scTextureSize);
        auto result = vkAllocateCommandBuffers(context->GetVkDevice(), &commandBufferAllocateInfo, commandBuffers.data());
    }
}

IntrusivePtr<VulkanTexture> VulkanRenderGroup::CreateAttachmentResource(VulkanSwapChain *swapChain, IntrusivePtr<AttachmentGraphNode> attachmentNode)
{
    auto texture = new VulkanTexture(context);
    auto textureUsage = DeferAttachmentUsage(attachmentNode);

    auto textureExtent = Texture::Extent{
        .width = swapChain->extent.width,
        .height = swapChain->extent.height,
        .depth = 1};

    auto textureConfig = Texture::Configuration{
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = 1,
        .tiling = Texture::Tiling::IMAGE_TILING_OPTIMAL};

    texture->Allocate(attachmentNode->format, textureUsage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureExtent, textureConfig);

    return texture;
}

void VulkanRenderGroup::prepareFrameBuffer(IntrusivePtr<VulkanGraphicPass> &renderPass, VulkanSwapChain *swapChain)
{
    auto vulkanRP = static_cast<VulkanGraphicPass *>(renderPass.get());

    auto &renderPassResource = this->renderPassResourceMap[renderPass];

    auto &attachmentImages = renderPassResource.attachmentImages;

    renderPassResource.frameBuffers.resize(swapChain->ImageSize());

    for (int i = 0; i < swapChain->ImageSize(); i++)
    {
        std::vector<VkImageView> attachmentViews;

        for (auto attachment : vulkanRP->attachmentNodes)
        {
            IntrusivePtr<VulkanTexture> texture;

            if (attachment->shared)
            {
                texture = (VulkanTexture *)(sharedResources[attachment->GlobalName()][i].get());
            }
            else if (groupScopeResources.count(attachment->GlobalName()) && groupScopeResources[attachment->GlobalName()].size() == swapChain->ImageSize())
            {
                texture = static_cast<VulkanTexture *>(groupScopeResources[attachment->GlobalName()][i].get());
            }
            else
            {
                texture = CreateAttachmentResource(swapChain, attachment);
                groupScopeResources[attachment->GlobalName()].push_back(texture);
            }

            VulkanAuxiliaryExecutor::ImageLayoutConfig config = {
                .aspectMask = VulkanRenderGroup::DeferAttachmentAspect(attachment),
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_GENERAL,
                .srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT};

            auto res = this->auxiliaryExecutor->SetImageLayout(texture, config);

            // spdlog::info("setting {} {} {} {}", i, attachment->name + ":" + attachment->passName, res, fmt::ptr(texture->GetImage()));

            VkImageAspectFlags imageAspect = {};

            auto textureUsage = DeferAttachmentUsage(attachment);

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

            IntrusivePtr<VulkanSampler> sampler;
            if (attachment->inputSubPassNames.size() != 0)
            {
                sampler = new VulkanSampler(context, texture);
                sampler->Allocate({});
            }

            attachmentImages[attachment->GlobalName()].push_back({texture, textureView, sampler});
            attachmentViews.push_back(textureView->GetImageView());
        }

        VkFramebufferCreateInfo frameBufferCreateInfo = {};
        frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferCreateInfo.renderPass = vulkanRP->GetRenderPass();
        frameBufferCreateInfo.attachmentCount = (uint32_t)attachmentViews.size();
        frameBufferCreateInfo.pAttachments = attachmentViews.data();
        frameBufferCreateInfo.width = swapChain->extent.width;
        frameBufferCreateInfo.height = swapChain->extent.height;
        frameBufferCreateInfo.layers = 1;

        VkFramebuffer frameBuffer;
        auto result = vkCreateFramebuffer(context->GetVkDevice(), &frameBufferCreateInfo, nullptr, &frameBuffer);
        renderPassResource.frameBuffers[i] = frameBuffer;
    }
}

void VulkanRenderGroup::prepareResources(IntrusivePtr<VulkanGraphicPass> &renderPass, VulkanSwapChain *swapChain)
{
    auto vulkanRP = static_cast<VulkanGraphicPass *>(renderPass.get());

    auto &renderPassResource = this->renderPassResourceMap[renderPass];

    auto &buffers = renderPassResource.buffers;

    renderPassResource.frameBuffers.resize(swapChain->ImageSize());

    for (int i = 0; i < swapChain->ImageSize(); i++)
    {

        
    }
}

void VulkanRenderGroup::resolveDrawStatesDescriptors(VulkanSwapChain *swapChain)
{
    for (auto &[pipeline, drawStates] : resourceBindingStates)
    {
        auto vulkanPL = static_cast<VulkanGraphicsPipeline *>(pipeline.get());
        auto vulkanRP = vulkanPL->GetRenderPass();

        auto subPassNode = vulkanRP->GetGraphicRenderPassGraphNode(vulkanPL->GetPipelineName());

        // check every drawState's set binding of pipeline
        // if the resourceName is in attachmentImages, then it's the internal resource
        for (auto &drawState : drawStates)
        {
            // check each set binding in subpass
            for (auto &[resourceName, bindingSet] : subPassNode->bindingSets)
            {
                spdlog::info("checking resource {} at {} {}", resourceName, bindingSet.set, bindingSet.binding);

                auto vulkanDrawState = static_cast<VulkanResourceBindingState *>(drawState.get());

                // for each frame, bind internal resource
                for (auto frameIndex = 0; frameIndex < swapChain->ImageSize(); frameIndex++)
                {
                    auto &resourceHandleMap = vulkanDrawState->GetDescriptorSet()->GetResourceHandlesMap(frameIndex);
                    // set empty slot only, no override
                    if (!resourceHandleMap[bindingSet.set][bindingSet.binding].Empty())
                        continue;

                    spdlog::info("resource: {} frame index {} set {} binding {} is empty", resourceName, frameIndex, bindingSet.set, bindingSet.binding);
                    // default resource (immutable) bind at frameIndex == 0
                    // per frame attachmentImages is prepared in prepareFrameBuffer
                    // per frame buffers is prepared in prepareResources
                    if (frameIndex == 0)
                    {
                        IntrusivePtr<ResourceHandle> resource;
                        if (bindingSet.type == GraphNode::ATTACHMENT)
                        {
                            resource = renderPassResourceMap[vulkanRP].attachmentImages[resourceName][frameIndex].sampler;
                        }
                        if (bindingSet.type == GraphNode::BUFFER)
                        {
                            resource = renderPassResourceMap[vulkanRP].buffers[resourceName][frameIndex];
                        }
                        vulkanDrawState->BindInternal(frameIndex, bindingSet.set, bindingSet.binding, resource);
                    }
                    else
                    {
                        // copy default (immutable) resource from frameIndex == 0
                        spdlog::info("copying from frame {} {} {}", frameIndex, bindingSet.set, bindingSet.binding);
                        vulkanDrawState->Copy(frameIndex, bindingSet.set, bindingSet.binding);
                    }
                }
            }
        }
    }
}
