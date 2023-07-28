#pragma once

#include <RHI/RHIRuntime.h>

class RuntimeEntry
{
public:
    enum class Type
    {
        VULKAN
    };

    RuntimeEntry(Type type) : type(type)
    {
    }

    IntrusivePtr<RHIRuntime> Create();

private:
    Type type;
};