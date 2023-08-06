#pragma once

#include <Core/IntrusivePtr.h>

// runtime descriptor for uniform binding
class UniformBinding : public IntrusiveCounter<UniformBinding>
{
public:
    UniformBinding() = default;
    virtual ~UniformBinding() = default;
};