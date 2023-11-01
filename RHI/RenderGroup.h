#pragma once

#include <Core/IntrusivePtr.h>

#include <RHI/ResourceBindingState.h>
#include <RHI/Pipeline.h>
#include <RHI/PipelineStates.h>

#include <FrameGraph/Graph.h>

// runtime struct of renderpass json
class RenderGroup : public IntrusiveCounter<RenderGroup>
{
public:
    RenderGroup(IntrusivePtr<Graph> graph);
    virtual ~RenderGroup() = default;
    virtual void Build() = 0;

    virtual IntrusivePtr<Pipeline> CreatePipeline(std::string subPassName, PipelineStates pipelineStates) = 0;

    virtual void AddBindingState(IntrusivePtr<ResourceBindingState> state) = 0;

    const IntrusivePtr<Graph> GetGraph();

    std::string Name();

protected:
    IntrusivePtr<Graph> graph;

    // subpass name -> graphic, compute pipeline
    std::unordered_map<std::string, IntrusivePtr<Pipeline>> pipelineMap;
};