#pragma once

#include <vector>

#include <Core/IntrusivePtr.h>
#include <RHI/ResourceHandle.h>
#include <RHI/Buffer.h>

class MutableBuffer : public ResourceHandle
{
public:
    MutableBuffer(IntrusivePtr<Buffer> defaultBuffer);
    void ReSize(uint32_t size);
    IntrusivePtr<Buffer>& GetBuffer(uint32_t index);
    void SetBuffer(uint32_t index, IntrusivePtr<Buffer> buffer);
    
protected:
    virtual ~MutableBuffer() = default;

    std::vector<IntrusivePtr<Buffer>> buffers;
};