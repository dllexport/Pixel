#pragma once

#include <Core/IntrusivePtr.h>

#include <FrameGraph/Graph.h>

#include <RHI/RenderPass.h>
#include <RHI/Pipeline.h>

class RHIRuntime : public IntrusiveCounter<RHIRuntime>
{
public:
    virtual IntrusivePtr<RenderPass> CreateRenderPass(IntrusivePtr<Graph> graph) = 0;
    virtual IntrusivePtr<Pipeline> CreatePipeline(IntrusivePtr<RenderPass>, std::string subPassName, PipelineStates pipelineStates) = 0;
};