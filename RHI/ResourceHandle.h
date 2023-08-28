#pragma once

#include <Core/IntrusivePtr.h>

class ResourceHandle : public IntrusiveCounter<ResourceHandle>
{
public:
    enum ResourceHandleType
    {
        BUFFER,
        BUFFER_ARRAY,
        TEXTURE,
        SAMPLER
    };

    // avoid dynamic_cast
    ResourceHandleType type;

    ResourceHandle(ResourceHandleType type) : type(type) {}
    virtual ~ResourceHandle() = default;
};