#pragma once

#include <RHI/PipelineStates.h>
#include <RHI/VulkanRuntime/Pipeline.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/ComputePass.h>
#include <RHI/VulkanRuntime/SPIVReflection.h>

#include <RHI/VulkanRuntime/PipelineLayout.h>

class VulkanComputePipeline : public VulkanPipeline
{
public:
    VulkanComputePipeline(IntrusivePtr<Context> context, IntrusivePtr<VulkanComputePass> computePass, std::string groupName, std::string pipelineName, ComputePipelineStates pipelineStates);
    virtual ~VulkanComputePipeline() override;
    virtual void Build() override;

    IntrusivePtr<VulkanPipelineLayout> &GetPipelineLayout();

    VkPipeline GetPipeline()
    {
        return pipeline;
    }

    IntrusivePtr<VulkanComputePass> GetComputePass()
    {
        return computePass;
    }

    virtual IntrusivePtr<RenderPassGraphNode> GetRenderPassGraphNode() override
    {
        return computePass->renderPassGraphNodes[0];
    }

private:
    ComputePipelineStates pipelineStates;
    IntrusivePtr<VulkanComputePass> computePass;
};