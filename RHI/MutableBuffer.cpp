#include <RHI/MutableBuffer.h>

MutableBuffer::MutableBuffer(IntrusivePtr<Buffer> defaultBuffer) : ResourceHandle(ResourceHandleType::BUFFER_ARRAY), buffers({defaultBuffer}) {}

void MutableBuffer::ReSize(uint32_t size)
{
    int currentSize = buffers.size();
    if (currentSize >= size)
    {
        return;
    }

    for (int i = currentSize; i < size; i++)
    {
        buffers.push_back(buffers[0]->Clone());
    }
}

IntrusivePtr<Buffer>& MutableBuffer::GetBuffer(uint32_t index)
{
    ReSize(index + 1);
    return buffers[index];
}

void MutableBuffer::SetBuffer(uint32_t index, IntrusivePtr<Buffer> buffer)
{
    ReSize(index + 1);
    buffers[index] = buffer;
}
