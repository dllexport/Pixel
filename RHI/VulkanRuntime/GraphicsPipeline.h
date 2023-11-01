#pragma once

#include <RHI/Pipeline.h>
#include <RHI/PipelineStates.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/SPIVReflection.h>

class VulkanPipelineLayout;
class VulkanGraphicsPipeline : public Pipeline
{
public:
    VulkanGraphicsPipeline(IntrusivePtr<Context> context, IntrusivePtr<VulkanRenderPass> renderPass, std::string groupName, std::string pipelineName, PipelineStates pipelineStates);
    virtual ~VulkanGraphicsPipeline() override;
    virtual void Build() override;

    IntrusivePtr<VulkanPipelineLayout> &GetPipelineLayout();

    VkPipeline GetPipeline();

    IntrusivePtr<VulkanRenderPass> GetRenderPass();

private:
    IntrusivePtr<Context> context;

    IntrusivePtr<VulkanRenderPass> renderPass;

    PipelineStates pipelineStates;

    std::vector<VkShaderModule> shaderModules;
    std::unordered_map<VkShaderStageFlagBits, std::vector<char>> shaderCode;
    std::vector<VkPipelineShaderStageCreateInfo> TranslateShaderState(ShaderState state);
    VkShaderModule loadShader(std::string path, VkShaderStageFlagBits stage);

    IntrusivePtr<VulkanPipelineLayout> pipelineLayout;

    VkPipeline pipeline;
};