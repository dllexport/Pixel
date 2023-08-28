#pragma once

#include <vector>

#include <Core/IntrusivePtr.h>
#include <RHI/ResourceHandle.h>
#include <RHI/Buffer.h>
#include <RHI/Memory.h>

class BufferArray : public ResourceHandle
{
public:
    BufferArray(IntrusivePtr<Buffer> defaultBuffer);
    void ReSize(uint32_t size);
    IntrusivePtr<Buffer> &GetBuffer(uint32_t index);

protected:
    virtual ~BufferArray() = default;

    std::vector<IntrusivePtr<Buffer>> buffers;
};