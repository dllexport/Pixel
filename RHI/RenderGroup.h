#pragma once

#include <Core/IntrusivePtr.h>

#include <RHI/ResourceBindingState.h>

#include <FrameGraph/Graph.h>

// runtime struct of renderpass json
class RenderGroup : public IntrusiveCounter<RenderGroup>
{
public:
    RenderGroup(IntrusivePtr<Graph> graph);
    virtual ~RenderGroup() = default;
    virtual void Build() = 0;

    const IntrusivePtr<Graph> GetGraph();

    virtual void AddBindingState(IntrusivePtr<ResourceBindingState> state) = 0;

    std::string Name();

protected:
    IntrusivePtr<Graph> graph;
};