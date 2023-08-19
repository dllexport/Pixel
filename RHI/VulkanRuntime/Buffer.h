#pragma once
#include <vulkan/vulkan.h>

#include <RHI/Buffer.h>
#include <RHI/VulkanRuntime/Context.h>

class VulkanBuffer : public Buffer
{
public:
    VulkanBuffer(IntrusivePtr<Context> context);
    virtual ~VulkanBuffer() override;

    virtual void *Map() override;

    VkBuffer &GetBuffer()
    {
        return buffer;
    }

    virtual size_t Size() override
    {
        return bufferAllocationInfo.size;
    }

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;

    VkBuffer buffer;
    VmaAllocation bufferAllocation;
    VmaAllocationInfo bufferAllocationInfo;
    void *mappedData = nullptr;

    virtual bool Allocate(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) override;
};
