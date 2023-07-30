#pragma once
#include <vulkan/vulkan.h>

#include <RHI/Buffer.h>
#include <RHI/VulkanRuntime/Context.h>

class VulkanBuffer : public Buffer
{
public:
    VulkanBuffer(IntrusivePtr<Context> context);

    virtual bool Allocate(Buffer::TypeBits type, Buffer::MemoryPropertyBits memoryProperties, uint32_t size) override;
    virtual void *Map() override;

private:
    IntrusivePtr<Context> context;

    VkBuffer buffer;
    VmaAllocation bufferAllocation;
    VmaAllocationInfo bufferAllocationInfo;
};
