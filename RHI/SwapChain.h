#pragma once

#include <Core/IntrusivePtr.h>

class SwapChain : public IntrusiveCounter<SwapChain>
{
public:
    SwapChain() = default;
    virtual ~SwapChain() = default;

    virtual uint32_t ImageSize() = 0;
    virtual uint32_t Acquire(uint32_t currentFrame) = 0;
    virtual bool Present(uint32_t index, uint32_t currentFrame) = 0;
};