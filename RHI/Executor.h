#pragma once

#include <Core/IntrusivePtr.h>

class Executor : public IntrusiveCounter<Executor>
{
public:
    virtual ~Executor() = default;

    // build command buffer
    virtual bool Execute() = 0;

    // wait gpu idle
    virtual void WaitIdle() = 0;
};