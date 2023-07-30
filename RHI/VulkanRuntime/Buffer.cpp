#include <RHI/VulkanRuntime/Buffer.h>

VulkanBuffer::VulkanBuffer(IntrusivePtr<Context> context) : context(context)
{
}

bool VulkanBuffer::Allocate(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size)
{
    auto allocator = context->GetVmaAllocator();
    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.usage = type;
    bufferCreateInfo.size = size;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = memoryProperties;

    auto result = vmaCreateBuffer(allocator, &bufferCreateInfo, &allocInfo, &buffer, &bufferAllocation, &bufferAllocationInfo);

    return result == VK_SUCCESS;
}

void *VulkanBuffer::Map()
{
    auto allocator = context->GetVmaAllocator();
    void *mappedData;
    auto result = vmaMapMemory(allocator, this->bufferAllocation, &mappedData);
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }
    return mappedData;
}
