#pragma once

#include <RHI/VulkanRuntime/Context.h>
#include <FrameGraph/Graph.h>
#include <FrameGraph/GraphNode.h>

#include <vulkan/vulkan.h>

class VulkanComputePass : public IntrusiveCounter<VulkanComputePass>
{
public:
    VulkanComputePass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph);
    ~VulkanComputePass() {}

    void Build(std::string pipelineName)
    {
        auto cn = this->graph->GetNodeMap().at(pipelineName);
        this->computePassNode = cn->As<ComputeRenderPassGraphNode*>();
    }

private:
    friend class VulkanComputePipeline;
    friend class VulkanGroupExecutor;
    IntrusivePtr<Context> context;
    IntrusivePtr<Graph> graph;
    IntrusivePtr<ComputeRenderPassGraphNode> computePassNode;
};