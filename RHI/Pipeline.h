#pragma once

#include <string>

#include <Core/IntrusivePtr.h>

class Pipeline : public IntrusiveCounter<Pipeline>
{
public:
    Pipeline();
    virtual ~Pipeline() = default;
    virtual void Build() = 0;

    std::string GetPipelineName()
    {
        return name;
    }

    std::string name;
};