#pragma once

#include <Core/IntrusivePtr.h>

#include <RHI/PipelineStates.h>

class Pipeline : public IntrusiveCounter<Pipeline>
{
public:
    Pipeline(PipelineStates pipelineStates);
    virtual void Build() = 0;

protected:
    PipelineStates pipelineStates;
};