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

    virtual IntrusivePtr<Buffer> Clone() override;

    virtual bool Allocate(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) override;
    bool Allocate(VkBufferCreateInfo bufferCI, VmaAllocationCreateInfo memoryCI);

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;

    VkBuffer buffer;
    VmaAllocation bufferAllocation;
    VmaAllocationInfo bufferAllocationInfo;
    void *mappedData = nullptr;

    VkBufferCreateInfo bufferCI;
    VmaAllocationCreateInfo memoryCI;
};
