#include <RHI/VulkanRuntime/RenderGroup.h>

#include <spdlog/spdlog.h>

#include <RHI/ConstantBuffer.h>

#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>

VulkanRenderGroup::VulkanRenderGroup(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph) : RenderGroup(graph), context(context)
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

struct SplitSubPassMeta
{
    std::string type;
    std::vector<std::string> nodes;
};

std::vector<SplitSubPassMeta>
SplitSubPasses(IntrusivePtr<Graph> graph)
{
    std::vector<SplitSubPassMeta> result;
    for (auto [level, passes] : graph->Topo().levelsRenderPassOnly)
    {
        for (auto pass : passes)
            spdlog::info("level {}, pass: {}", level, pass->name);
    }
    return result;
}

void VulkanRenderGroup::Build()
{

    // for (auto &[rp, _] : renderPassResourceMap)
    // {
    //     rp->Build();
    // }

    // for (auto &[rp, _] : computePassResourceMap)
    // {
    //     rp->Build();
    // }
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

        for (auto &graphicCommandBuffer : resource.commandBuffers)
        {
            vkResetCommandBuffer(graphicCommandBuffer, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
        }
        resource.commandBuffers.clear();
    }

    for (auto &[pipeline, drawStates] : resourceBindingStates)
    {
        for (auto &drawState : drawStates)
        {
            static_cast<VulkanResourceBindingState *>(drawState.get())->GetDescriptorSet()->ClearInternal();
        }
    }
}

void VulkanRenderGroup::AddBindingState(IntrusivePtr<ResourceBindingState> state)
{
    auto vgp = static_cast<VulkanGraphicsPipeline *>(state->GetPipeline().get());
    this->resourceBindingStates[vgp].push_back(static_cast<VulkanResourceBindingState *>(state.get()));
}

void VulkanRenderGroup::Prepare(VulkanSwapChain *swapChain)
{
    for (auto &[name, renderPass] : renderPasses)
    {
        prepareCommandBuffer(renderPass, swapChain);
        prepareFrameBuffer(renderPass, swapChain);
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

std::vector<VkCommandBuffer> VulkanRenderGroup::GetCommandBuffer(uint32_t currentImageIndex)
{
    std::vector<VkCommandBuffer> result;
    for (auto [_, cbs] : renderPassResourceMap)
    {
        result.push_back(cbs.commandBuffers[currentImageIndex]);
    }
    return result;
}

IntrusivePtr<VulkanRenderPass> VulkanRenderGroup::GetRenderPass(std::string name)
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
    for (auto &[name, renderPass] : renderPasses)
    {
        auto vulkanRP = static_cast<VulkanRenderPass *>(renderPass.get());
        auto &renderPassResource = renderPassResourceMap[renderPass];
        auto &commandBuffer = renderPassResource.commandBuffers[imageIndex];

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

        std::unordered_map<std::string, IntrusivePtr<VulkanGraphicsPipeline>> subPassName2Pipeline;

        for (auto [pipeline, resourceBinding] : resourceBindingStates)
        {
            auto vulkanPL = static_cast<VulkanGraphicsPipeline *>(pipeline.get());
            subPassName2Pipeline[vulkanPL->GetPipelineName()] = vulkanPL;
        }

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
                    scissor.extent = swapchain->extent;
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

void VulkanRenderGroup::prepareCommandBuffer(IntrusivePtr<VulkanRenderPass> &renderPass, VulkanSwapChain *swapChain)
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

void VulkanRenderGroup::prepareFrameBuffer(IntrusivePtr<VulkanRenderPass> &renderPass, VulkanSwapChain *swapChain)
{
    auto vulkanRP = static_cast<VulkanRenderPass *>(renderPass.get());

    auto &renderPassResource = this->renderPassResourceMap[renderPass];

    auto &attachmentImages = renderPassResource.attachmentImages;

    renderPassResource.frameBuffers.resize(swapChain->GetTextures().size());

    for (int i = 0; i < swapChain->GetTextures().size(); i++)
    {
        std::vector<VkImageView> attachmentViews;

        for (auto attachment : vulkanRP->attachmentNodes)
        {
            // if attachment is swapchain, use reference instead of creating one
            if (attachment->swapChain)
            {
                attachmentImages[attachment->name].push_back({swapChain->GetTextures()[i], swapChain->GetTextureViews()[i], nullptr});
                attachmentViews.push_back(swapChain->GetTextureViews()[i]->GetImageView());
                continue;
            }

            IntrusivePtr<VulkanTexture> texture = new VulkanTexture(context);
            auto textureExtent = Texture::Extent{
                .width = swapChain->extent.width,
                .height = swapChain->extent.height,
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
        frameBufferCreateInfo.width = swapChain->extent.width;
        frameBufferCreateInfo.height = swapChain->extent.height;
        frameBufferCreateInfo.layers = 1;

        VkFramebuffer frameBuffer;
        auto result = vkCreateFramebuffer(context->GetVkDevice(), &frameBufferCreateInfo, nullptr, &frameBuffer);
        renderPassResource.frameBuffers[i] = frameBuffer;
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

                    spdlog::info("frame index {} is empty", frameIndex);
                    // default resource (immutable) bind at frameIndex == 0
                    // per frame attachmentImages is prepared in prepareFrameBuffer
                    if (frameIndex == 0 || bindingSet.type == GraphNode::ATTACHMENT)
                    {
                        auto &attachmentImages = renderPassResourceMap[vulkanRP].attachmentImages[resourceName];
                        vulkanDrawState->BindInternal(frameIndex, bindingSet.set, bindingSet.binding, attachmentImages[frameIndex].sampler);
                    }
                    else
                    {
                        // copy default (immutable) resource from frameIndex == 0
                        vulkanDrawState->Copy(frameIndex, bindingSet.set, bindingSet.binding);
                    }
                }
            }
        }
    }
}