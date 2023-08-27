#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/SPIVReflection.h>

class VulkanPipelineLayout : public IntrusiveCounter<VulkanPipelineLayout>
{
public:
    VulkanPipelineLayout(IntrusivePtr<Context> context);
    ~VulkanPipelineLayout();

    void Build(std::vector<IntrusivePtr<SPIVReflection>> reflections);

    void ParseFromReflect(std::vector<char> spirv_code);

    VkPipelineLayout GetLayout()
    {
        return pipelineLayout;
    }

    auto &GetBindingsInSets()
    {
        return bindingsInSets;
    }

    auto &GetSetsLayouts()
    {
        return desriptorSetLayouts;
    }

    auto &GetSetBindingQueryMap()
    {
        return uniformNameBindingMap;
    }

private:
    IntrusivePtr<Context> context;
    std::vector<VkDescriptorSetLayout> desriptorSetLayouts;
    VkPipelineLayout pipelineLayout;

    std::vector<VkPushConstantRange> pushConstantRanges;
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindingsInSets;

    struct SetBindingQuery
    {
        uint32_t set;
        uint32_t binding;
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
    };

    std::unordered_multimap<std::string, SetBindingQuery> uniformNameBindingMap;
};