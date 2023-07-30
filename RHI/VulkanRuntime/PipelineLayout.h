#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/SPIVReflection.h>

class VulkanPipelineLayout
{
public:
    VulkanPipelineLayout(IntrusivePtr<Context> context) : context(context)
    {
    }

    void Build(std::vector<IntrusivePtr<SPIVReflection>> reflections);

    void ParseFromReflect(std::vector<char> spirv_code);

    VkPipelineLayout GetLayout()
    {
        return pipelineLayout;
    }

private:
    IntrusivePtr<Context> context;
    std::vector<VkDescriptorSetLayout> desriptorSetLayouts;
    VkPipelineLayout pipelineLayout;

    std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSets;
    std::unordered_map<std::string, VkDescriptorSetLayoutBinding> vertexShaderDescriptorBindingMap;
    std::unordered_map<std::string, VkDescriptorSetLayoutBinding> fragmentShaderDescriptorBindingMap;
};