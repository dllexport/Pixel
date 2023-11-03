#pragma once

#include <RHI/PipelineStates.h>

#include <RHI/VulkanRuntime/Pipeline.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/GraphicPass.h>
#include <RHI/VulkanRuntime/SPIVReflection.h>

class VulkanPipelineLayout;
class VulkanGraphicsPipeline : public VulkanPipeline
{
public:
    VulkanGraphicsPipeline(IntrusivePtr<Context> context, IntrusivePtr<VulkanGraphicPass> renderPass, std::string groupName, std::string pipelineName, PipelineStates pipelineStates);
    virtual ~VulkanGraphicsPipeline() override;
    virtual void Build() override;

    IntrusivePtr<VulkanPipelineLayout> &GetPipelineLayout();

    VkPipeline GetPipeline();

    IntrusivePtr<VulkanGraphicPass> GetRenderPass();

private:
    IntrusivePtr<VulkanGraphicPass> renderPass;

    PipelineStates pipelineStates;
};