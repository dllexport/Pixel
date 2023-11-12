#pragma once

#include <Core/IntrusivePtr.h>
#include <vector>

#include <RHI/VulkanRuntime/Context.h>
#include <FrameGraph/Graph.h>

class VulkanPass : public IntrusiveCounter<VulkanPass>
{
public:
    VulkanPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph) : context(context), graph(graph) {}
    virtual ~VulkanPass() = default;

    std::vector<IntrusivePtr<RenderPassGraphNode>> &GetRenderPassGraphNode()
    {
        return renderPassGraphNodes;
    }

protected:
    IntrusivePtr<Context> context;
    IntrusivePtr<Graph> graph;

    // for graphic pass, ordered subpass node from 0 to n
    // for compute pass, renderPasses.size() == 1
    std::vector<IntrusivePtr<RenderPassGraphNode>> renderPassGraphNodes;
};