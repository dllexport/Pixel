#pragma once

#include <RHI/RenderPassExecutor.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Image.h>

#include <vulkan/vulkan.h>

class VulkanRenderPassExecutor : public RenderPassExecutor
{
public:
    VulkanRenderPassExecutor(IntrusivePtr<Context> context);
    virtual ~VulkanRenderPassExecutor() override;

    virtual void Prepare() override;
    virtual bool Execute() override;
    // rebuild command buffer
    virtual void Update() override;
    virtual void WaitIdle() override;
    virtual void Reset() override;

private:
    IntrusivePtr<Context> context;
    VkCommandPool graphicCommandPool;
    VkCommandPool computeCommandPool;

    struct RenderPassResource
    {
        // attachment name -> vulkan image(1 per inflight)
        std::unordered_map<std::string, std::vector<VulkanImage>> attachmentImages;
        std::vector<VkFramebuffer> frameBuffers;
        std::vector<VkCommandBuffer> graphicCommandBuffers;
    };

    std::unordered_map<IntrusivePtr<RenderPass>, RenderPassResource> renderPassResourceMap;
    std::vector<VkFence> queueCompleteFences;

    void prepareFences();
    void prepareCommandPool();
    void buildCommandBuffer(uint32_t imageIndex);
    void prepareRenderPassResource(IntrusivePtr<RenderPass>& renderPass);
    void prepareCommandBuffer(IntrusivePtr<RenderPass>& renderPass);
    void prepareFrameBuffer(IntrusivePtr<RenderPass>& renderPass);

    void resolveDrawStatesDescriptors();

    uint64_t currentFrame = 0;
};