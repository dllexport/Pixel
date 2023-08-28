#pragma once

#include <memory>

#include <RHI/Buffer.h>

class ConstantBuffer : public Buffer
{
public:
    ConstantBuffer(uint32_t size) : buffer(size)
    {
    }

    virtual ~ConstantBuffer() override
    {
    }

    virtual bool Allocate(TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) override
    {
        return true;
    }

    virtual void *Map() override
    {
        return buffer.data();
    }

    virtual size_t Size() override
    {
        return buffer.size();
    }

    virtual IntrusivePtr<Buffer> Clone() override
    {
        auto newBuffer = new ConstantBuffer(this->buffer.size());
        return newBuffer;
    }

private:
    std::vector<char> buffer;
};