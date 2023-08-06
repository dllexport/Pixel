#pragma once

#include <Core/IntrusivePtr.h>

#include <RHI/PipelineStates.h>
#include <RHI/RenderPass.h>

class Pipeline : public IntrusiveCounter<Pipeline>
{
public:
    Pipeline(PipelineStates pipelineStates);
    virtual ~Pipeline() = default;
    virtual void Build() = 0;
    IntrusivePtr<RenderPass> GetRenderPass();

protected:
    PipelineStates pipelineStates;
    IntrusivePtr<RenderPass> renderPass;
};