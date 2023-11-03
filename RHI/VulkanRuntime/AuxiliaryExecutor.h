#pragma once

#include <RHI/AuxiliaryExecutor.h>
#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <vulkan/vulkan.h>

class VulkanAuxiliaryExecutor : public AuxiliaryExecutor
{
public:
    VulkanAuxiliaryExecutor(IntrusivePtr<Context> context);
    virtual ~VulkanAuxiliaryExecutor();

    // build command buffer
    virtual bool Execute() override;

    // wait gpu idle
    virtual void WaitIdle() override;

    virtual void TransferResource(IntrusivePtr<Texture> gpuTexture, IntrusivePtr<Buffer> hostBuffer, TransferConfig config = {}) override;
    virtual void TransferResource(IntrusivePtr<Buffer> gpuBuffer, IntrusivePtr<Buffer> hostBuffer) override;

    virtual void Reset() override;

    struct ImageLayoutConfig
    {
        VkImageAspectFlags aspectMask;

        VkImageLayout oldLayout;
        VkImageLayout newLayout;

        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
    };
    
    bool SetImageLayout(IntrusivePtr<VulkanTexture> texture, ImageLayoutConfig config);

private:
    IntrusivePtr<Context> context;

    VkCommandPool graphicCommandPool;
    VkCommandPool computeCommandPool;

    struct SubmitGroup
    {
        VkCommandBuffer commandBuffer;

        // ptr copy to keep resource alive during submit
        IntrusivePtr<Texture> dstTexture;
        IntrusivePtr<Buffer> dstBuffer;
        IntrusivePtr<Buffer> srcBuffer;
    };

    // commandBuffers to be submit
    std::vector<SubmitGroup> submitGroups;

    void prepareCommandPool();
    void resetCommandPool();
    VkCommandBuffer allocateCommandBuffer(VkCommandPool pool);
};