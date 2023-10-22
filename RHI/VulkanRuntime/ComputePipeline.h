#pragma once

#include <RHI/Pipeline.h>
#include <RHI/PipelineStates.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/RenderGroup.h>
#include <RHI/VulkanRuntime/SPIVReflection.h>

#include <RHI/VulkanRuntime/PipelineLayout.h>

class VulkanComputePipeline : public Pipeline
{
public:
    VulkanComputePipeline(IntrusivePtr<Context> context, IntrusivePtr<RenderGroup> renderGroup, std::string passName);
    virtual ~VulkanComputePipeline() override;
    virtual void Build() override;

    IntrusivePtr<VulkanPipelineLayout> &GetPipelineLayout();

    VkPipeline GetPipeline()
    {
        return pipeline;
    }

private:
    IntrusivePtr<Context> context;
    std::string subPassName;

    std::vector<VkShaderModule> shaderModules;
    std::unordered_map<VkShaderStageFlagBits, std::vector<char>> shaderCode;
    std::vector<VkPipelineShaderStageCreateInfo> TranslateShaderState(ShaderState state);
    VkShaderModule loadShader(std::string path, VkShaderStageFlagBits stage);

    IntrusivePtr<VulkanPipelineLayout> pipelineLayout;

    VkPipeline pipeline;
};