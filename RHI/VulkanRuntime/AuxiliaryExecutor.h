#pragma once

#include <RHI/AuxiliaryExecutor.h>
#include <RHI/VulkanRuntime/Context.h>

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

    virtual void TransferResource(IntrusivePtr<Texture> gpuTexture, IntrusivePtr<Buffer> hostBuffer) override;
    virtual void TransferResource(IntrusivePtr<Buffer> gpuBuffer, IntrusivePtr<Buffer> hostBuffer) override;

private:
    IntrusivePtr<Context> context;

    VkCommandPool graphicCommandPool;
    VkCommandPool computeCommandPool;

    struct SubmitGroup
    {
        VkCommandBuffer commandBuffer;

        // ptr copy to keep resource alive during submit
        IntrusivePtr<Texture> texture;
        IntrusivePtr<Buffer> buffer;
    };

    // commandBuffers to be submit
    std::vector<SubmitGroup> submitGroups;

    void prepareCommandPool();
    VkCommandBuffer allocateCommandBuffer(VkCommandPool pool);
};