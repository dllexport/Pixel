#pragma once

#include <RHI/Pipeline.h>
#include <RHI/VulkanRuntime/ComputePipeline.h>
#include <RHI/VulkanRuntime/Context.h>
#include <FrameGraph/Graph.h>
#include <FrameGraph/GraphNode.h>

#include <vulkan/vulkan.h>

class VulkanComputePass : public IntrusiveCounter<VulkanComputePass>
{
public:
    VulkanComputePass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph);
    ~VulkanComputePass() {}

    void Build() {}

private:
    friend class VulkanComputePipeline;
    friend class VulkanGroupExecutor;
    IntrusivePtr<Context> context;
    IntrusivePtr<Graph> graph;
    IntrusivePtr<ComputeRenderPassGraphNode> computePassNode;
};