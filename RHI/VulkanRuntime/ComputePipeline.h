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
    VulkanComputePipeline(IntrusivePtr<Context> context, IntrusivePtr<VulkanComputePass> computePass, std::string pipelineName, std::string groupName, ComputePipelineStates pipelineStates);
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

private:
    std::string subPassName;

    ComputePipelineStates pipelineStates;
    IntrusivePtr<VulkanComputePass> computePass;
};