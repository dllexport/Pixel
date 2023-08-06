#pragma once

#include <RHI/RenderPassExecutor.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>

#include <vulkan/vulkan.h>

class VulkanRenderPassExecutor : public RenderPassExecutor
{
public:
    VulkanRenderPassExecutor(IntrusivePtr<Context> context, IntrusivePtr<RenderPass> renderPass);
    virtual ~VulkanRenderPassExecutor() override;

    virtual void Import(IntrusivePtr<ResourceHandle> resource) override;
    virtual void Prepare() override;
    virtual void Execute() override;

private:
    IntrusivePtr<Context> context;
    VkCommandPool graphicCommandPool;
    VkCommandPool computeCommandPool;
    std::vector<VkCommandBuffer> graphicCommandBuffers;

    std::unordered_set<IntrusivePtr<ResourceHandle>> externalAttachments;
    std::vector<IntrusivePtr<VulkanTexture>> attachmentTextures;
    std::vector<IntrusivePtr<VulkanTextureView>> attachmentTextureViews;
    VkFramebuffer frameBuffer;

    void prepareCommandPool();
    void prepareCommandBuffer();
    void buildCommandBuffer();
};