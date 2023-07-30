#pragma once
#include <vulkan/vulkan.h>

#include <RHI/Buffer.h>
#include <RHI/VulkanRuntime/Context.h>

class VulkanBuffer : public Buffer
{
public:
    VulkanBuffer(IntrusivePtr<Context> context);

    virtual void *Map() override;

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;

    VkBuffer buffer;
    VmaAllocation bufferAllocation;
    VmaAllocationInfo bufferAllocationInfo;

    virtual bool Allocate(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) override;
};
