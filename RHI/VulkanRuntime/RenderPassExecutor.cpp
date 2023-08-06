#include <RHI/VulkanRuntime/RenderPassExecutor.h>
#include <RHI/VulkanRuntime/ResourceBindingState.h>

#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/RenderPass.h>

#include <FrameGraph/Graph.h>

#include <spdlog/spdlog.h>

VulkanRenderPassExecutor::VulkanRenderPassExecutor(IntrusivePtr<Context> context, IntrusivePtr<RenderPass> renderPass) : RenderPassExecutor(renderPass), context(context)
{
    prepareCommandPool();
    prepareCommandBuffer();
}

VulkanRenderPassExecutor::~VulkanRenderPassExecutor()
{
    vkDestroyCommandPool(context->GetVkDevice(), graphicCommandPool, nullptr);
    vkDestroyCommandPool(context->GetVkDevice(), computeCommandPool, nullptr);
    vkDestroyFramebuffer(context->GetVkDevice(), frameBuffer, nullptr);
}

void VulkanRenderPassExecutor::Import(IntrusivePtr<ResourceHandle> resource)
{
    externalAttachments.insert(resource);
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
    graphicCommandBuffers.resize(2);
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = graphicCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 2;
    auto result = vkAllocateCommandBuffers(context->GetVkDevice(), &commandBufferAllocateInfo, graphicCommandBuffers.data());
}

void VulkanRenderPassExecutor::buildCommandBuffer()
{

    auto vulkanRP = static_cast<VulkanRenderPass *>(this->renderPass.get());

    VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
    cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    std::vector<VkClearValue> clearValues(vulkanRP->attachmentMiniDescriptions.size());
    clearValues[0].depthStencil = {1.0f, 0};
    clearValues[1].color = {{0.0f, 0.0f, 0.0f, 0.0f}};

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = vulkanRP->GetRenderPass();
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent = VkExtent2D{swapChainExtent.width, swapChainExtent.height};
    renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();
    renderPassBeginInfo.framebuffer = frameBuffer;

    auto graphicCommandBuffer = graphicCommandBuffers[0];

    vkBeginCommandBuffer(graphicCommandBuffers[0], &cmdBufferBeginInfo);

    vkCmdBeginRenderPass(graphicCommandBuffers[0], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.width = swapChainExtent.width;
    viewport.height = swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(graphicCommandBuffers[0], 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.extent.width = swapChainExtent.width;
    scissor.extent.height = swapChainExtent.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(graphicCommandBuffers[0], 0, 1, &scissor);

    std::unordered_map<std::string, IntrusivePtr<VulkanPipeline>> subPassName2Pipeline;

    for (auto [pipeline, resourceBinding] : resourceBindingStates)
    {
        auto vulkanPL = static_cast<VulkanPipeline *>(pipeline.get());
        subPassName2Pipeline[vulkanPL->GetSubPassName()] = vulkanPL;
    }

    auto graph = this->renderPass->GetGraph();
    auto topoResult = graph->Topo();
    for (auto &[k, rps] : topoResult.levelsRenderPassOnly)
    {
        spdlog::info("level: {}", k);
        for (auto &node : rps)
        {
            spdlog::info("{}", node->name);
            auto vulkanPL = subPassName2Pipeline[node->name];
            auto vrbs = resourceBindingStates[vulkanPL];
            for (auto vrb : vrbs)
            {
                auto vulkanRBS = static_cast<VulkanResourceBindingState *>(vrb.get());
                auto pipelineLayout = vulkanPL->GetPipelineLayout();
                auto descriptorSets = vulkanRBS->GetDescriptorSets();

                vkCmdBindPipeline(graphicCommandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPL->GetPipeline());
                vkCmdBindDescriptorSets(graphicCommandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->GetLayout(), 0, descriptorSets.size(), descriptorSets.data(), 0, nullptr);

                const VkDeviceSize offsets[1] = {0};
                vkCmdBindVertexBuffers(graphicCommandBuffers[0], 0, 1, &vulkanRBS->GetVertexBuffer()->GetBuffer(), offsets);
                vkCmdBindIndexBuffer(graphicCommandBuffers[0], vulkanRBS->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
            }
        }
    }

    vkCmdEndRenderPass(graphicCommandBuffers[0]);

    vkEndCommandBuffer(graphicCommandBuffers[0]);
}

void VulkanRenderPassExecutor::Prepare()
{
    std::vector<VkImageView> attachmentViews;

    auto vulkanRP = static_cast<VulkanRenderPass *>(this->renderPass.get());

    for (auto attachment : vulkanRP->attachmentMiniDescriptions)
    {
        IntrusivePtr<VulkanTexture> texture = new VulkanTexture(context);
        auto textureExtent = Texture::Extent{
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .depth = 1};

        auto textureConfig = Texture::Configuration{
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = 1,
            .tiling = Texture::Tiling::IMAGE_TILING_OPTIMAL};

        // VkImageFormatProperties pp;
        // vkGetPhysicalDeviceImageFormatProperties(context->GetVkPhysicalDevice(), attachment.format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, attachment.usage, 0, &pp);

        texture->Allocate(VkFormatToGeneral(attachment.format), attachment.usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureExtent, textureConfig);

        attachmentTextures.push_back(texture);

        VkImageAspectFlags imageAspect = {};

        if (attachment.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            imageAspect |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (attachment.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            imageAspect |= VK_IMAGE_ASPECT_COLOR_BIT;
        }
        VkImageViewCreateInfo imageView = {};
        imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageView.format = attachment.format;
        imageView.subresourceRange = {};
        imageView.subresourceRange.aspectMask = imageAspect;
        imageView.subresourceRange.baseMipLevel = 0;
        imageView.subresourceRange.levelCount = 1;
        imageView.subresourceRange.baseArrayLayer = 0;
        imageView.subresourceRange.layerCount = 1;
        imageView.image = texture->GetImage();

        auto textureView = texture->CreateTextureView(imageView);
        attachmentTextureViews.push_back(textureView);

        attachmentViews.push_back(textureView->GetImageView());
    }

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.renderPass = vulkanRP->GetRenderPass();
    frameBufferCreateInfo.attachmentCount = (uint32_t)attachmentViews.size();
    frameBufferCreateInfo.pAttachments = attachmentViews.data();
    frameBufferCreateInfo.width = swapChainExtent.width;
    frameBufferCreateInfo.height = swapChainExtent.height;
    frameBufferCreateInfo.layers = 1;

    auto result = vkCreateFramebuffer(context->GetVkDevice(), &frameBufferCreateInfo, nullptr, &frameBuffer);

    buildCommandBuffer();
}

void VulkanRenderPassExecutor::Execute() {}