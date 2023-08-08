#pragma once

#include <RHI/RenderPassExecutor.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>

#include <vulkan/vulkan.h>

class VulkanRenderPassExecutor : public RenderPassExecutor
{
public:
    VulkanRenderPassExecutor(IntrusivePtr<Context> context);
    virtual ~VulkanRenderPassExecutor() override;

    virtual void Prepare() override;
    virtual void Execute() override;

private:
    IntrusivePtr<Context> context;
    VkCommandPool graphicCommandPool;
    VkCommandPool computeCommandPool;

    std::vector<std::vector<IntrusivePtr<VulkanTexture>>> attachmentTextures;
    std::vector<std::vector<IntrusivePtr<VulkanTextureView>>> attachmentTextureViews;

    std::unordered_map<IntrusivePtr<RenderPass>, std::vector<VkFramebuffer>> frameBuffers;
    std::unordered_map<IntrusivePtr<RenderPass>, std::vector<VkCommandBuffer>> graphicCommandBuffers;

    std::unordered_map<std::string, std::vector<VulkanTextureViewPair>> sharedTextureViews;

    void prepareCommandPool();
    void prepareCommandBuffer();
    void buildCommandBuffer(uint32_t imageIndex);

    uint64_t currentFrame = 0;
};