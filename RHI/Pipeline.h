#pragma once

#include <string>

#include <Core/IntrusivePtr.h>

class Pipeline : public IntrusiveCounter<Pipeline>
{
public:
    Pipeline(std::string groupName, std::string pipelineName);
    virtual ~Pipeline() = default;
    virtual void Build() = 0;

    std::string GetPipelineName()
    {
        return pipelineName;
    }

    std::string pipelineName;
    std::string groupName;
};