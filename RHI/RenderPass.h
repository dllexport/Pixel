#pragma once

#include <Core/IntrusivePtr.h>

#include <FrameGraph/Graph.h>

// template only
// doesn't hold actual resource
class RenderPass : public IntrusiveCounter<RenderPass>
{
public:
    RenderPass(IntrusivePtr<Graph> graph);
    virtual ~RenderPass() = default;
    virtual void Build() = 0;

    const IntrusivePtr<Graph> GetGraph()
    {
        return graph;
    }

    std::string Name();

protected:
    IntrusivePtr<Graph> graph;
};