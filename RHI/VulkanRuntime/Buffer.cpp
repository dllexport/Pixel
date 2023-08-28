#include <RHI/VulkanRuntime/Buffer.h>

VulkanBuffer::VulkanBuffer(IntrusivePtr<Context> context) : context(context)
{
}

VulkanBuffer::~VulkanBuffer()
{
    auto allocator = context->GetVmaAllocator();
    if (this->mappedData)
    {
        vmaUnmapMemory(allocator, bufferAllocation);
    }
    vmaDestroyBuffer(allocator, buffer, bufferAllocation);
}

bool VulkanBuffer::Allocate(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size)
{
    bufferCI = {};
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.usage = type;
    bufferCI.size = size;

    memoryCI = {};
    memoryCI.requiredFlags = memoryProperties;

    auto result = vmaCreateBuffer(context->GetVmaAllocator(), &ci, &memoryCI, &buffer, &bufferAllocation, &bufferAllocationInfo);

    return result == VK_SUCCESS;
}

bool VulkanBuffer::Allocate(VkBufferCreateInfo bufferCI, VmaAllocationCreateInfo memoryCI)
{
    auto result = vmaCreateBuffer(context->GetVmaAllocator(), &bufferCI, &memoryCI, &buffer, &bufferAllocation, &bufferAllocationInfo);
    return result == VK_SUCCESS;
}

IntrusivePtr<Buffer> VulkanBuffer::Clone()
{
    auto newBuffer = new VulkanBuffer(context);
    newBuffer->Allocate(bufferCI, memoryCI);
    return newBuffer;
}

void *VulkanBuffer::Map()
{
    if (mappedData)
    {
        return mappedData;
    }

    auto allocator = context->GetVmaAllocator();
    auto result = vmaMapMemory(allocator, this->bufferAllocation, &mappedData);
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }
    return mappedData;
}
