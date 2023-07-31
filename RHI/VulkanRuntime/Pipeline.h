#pragma once

#include <RHI/Pipeline.h>
#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/SPIVReflection.h>

class VulkanPipeline : public Pipeline
{
public:
    VulkanPipeline(IntrusivePtr<Context> context, IntrusivePtr<RenderPass> renderPass, std::string subPassName, PipelineStates pipelineStates);
    virtual ~VulkanPipeline() override;
    virtual void Build() override;

private:
    IntrusivePtr<Context> context;
    IntrusivePtr<VulkanRenderPass> renderPass;
    std::string subPassName;

    std::vector<VkShaderModule> shaderModules;
    std::unordered_map<VkShaderStageFlagBits, std::vector<char>> shaderCode;
    std::vector<VkPipelineShaderStageCreateInfo> TranslateShaderState(ShaderState state);
    VkShaderModule loadShader(std::string path, VkShaderStageFlagBits stage);

    VulkanPipelineLayout pipelineLayout;

    VkPipeline pipeline;
};