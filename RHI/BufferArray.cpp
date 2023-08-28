#include <RHI/BufferArray.h>

BufferArray::BufferArray(IntrusivePtr<Buffer> defaultBuffer) : ResourceHandle(ResourceHandleType::BUFFER_ARRAY), buffers({defaultBuffer}) {}

void BufferArray::ReSize(uint32_t size)
{
    int currentSize = buffers.size();
    if (currentSize > size)
    {
        buffers.resize(size);
        return;
    }

    for (int i = currentSize; i < size; i++)
    {
        buffers.push_back(buffers[0]->Clone());
    }
}

IntrusivePtr<Buffer> &BufferArray::GetBuffer(uint32_t index)
{
    return buffers[index];
}
