#pragma once

#include <Core/IntrusivePtr.h>

#include <FrameGraph/Graph.h>

// runtime struct of renderpass json
class RenderGroup : public IntrusiveCounter<RenderGroup>
{
public:
    RenderGroup(IntrusivePtr<Graph> graph);
    virtual ~RenderGroup() = default;
    virtual void Build() = 0;

    const IntrusivePtr<Graph> GetGraph();

    std::string Name();

protected:
    IntrusivePtr<Graph> graph;
};