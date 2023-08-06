#pragma once

#include <Core/IntrusivePtr.h>

class ResourceHandle : public IntrusiveCounter<ResourceHandle>
{
public:
    virtual ~ResourceHandle() = default;
};