#pragma once

#include <vector>
#include <string>

#include <Core/IntrusivePtr.h>
#include <Core/ReadFile.h>

#include <RHI/Pipeline.h>
#include <RHI/PipelineStates.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>

#include <vulkan/vulkan.h>

class VulkanPipeline : public Pipeline
{
public:
    VulkanPipeline(IntrusivePtr<Context> context, std::string groupName, std::string pipelineName);
    virtual void Build() = 0;

protected:
    IntrusivePtr<Context> context;
    std::vector<VkShaderModule> shaderModules;
    std::unordered_map<VkShaderStageFlagBits, std::vector<char>> shaderCode;
    std::vector<VkPipelineShaderStageCreateInfo> TranslateShaderState(ShaderState state);
    VkShaderModule loadShader(std::string path, VkShaderStageFlagBits stage);

    IntrusivePtr<VulkanPipelineLayout> pipelineLayout;

    VkPipeline pipeline;
};