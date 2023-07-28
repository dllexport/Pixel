#pragma once

#include <FrameGraph/Graph.h>

// template only
// doesn't hold actual resource
class RenderPass
{
public:
    RenderPass(IntrusivePtr<Graph> graph) : graph(graph) {

    }

    virtual void Build() = 0;
    IntrusivePtr<Graph> graph;
};