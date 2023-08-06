#pragma once

#include <Core/IntrusivePtr.h>

class SwapChain : public IntrusiveCounter<SwapChain>
{
public:
    SwapChain() = default;
    virtual ~SwapChain() = default;
    // virtual void Build() = 0;
};