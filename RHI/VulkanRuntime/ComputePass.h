#pragma once

#include <RHI/VulkanRuntime/Context.h>
#include <FrameGraph/Graph.h>
#include <FrameGraph/GraphNode.h>
#include <RHI/VulkanRuntime/Pass.h>

#include <vulkan/vulkan.h>

class VulkanComputePass : public VulkanPass
{
public:
    VulkanComputePass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph);
    virtual ~VulkanComputePass() override {}

    void Build(std::string pipelineName)
    {
        auto cn = this->graph->GetNodeMap().at(pipelineName);
        this->renderPassGraphNodes.push_back(cn->As<RenderPassGraphNode *>());
    }

private:
    friend class VulkanComputePipeline;
    friend class VulkanGroupExecutor;
};