#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <spirv_reflect.h>

#include <Core/IntrusivePtr.h>

struct VkDescriptorSetLayoutBindingWithName : VkDescriptorSetLayoutBinding
{
    std::string name;
};

class SPIVReflection : public IntrusiveCounter<SPIVReflection>
{
public:
    SPIVReflection(std::vector<char> shaderCode);
    ~SPIVReflection();

    struct InputVertexState
    {
        std::vector<VkVertexInputBindingDescription> inputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions;
    };

    InputVertexState ParseInputVertexState();

    struct DescriptorLayoutState
    {
        VkPushConstantRange pushConstantRange;
        // each set has it's array of VkDescriptorSetLayoutBinding
        std::vector<std::vector<VkDescriptorSetLayoutBindingWithName>> descriptorSetLayoutSets;
    };
    DescriptorLayoutState ParseDescriptorLayoutState();

    static VkDescriptorType TranslateReflectDescriptorType(SpvReflectDescriptorType type);
    static VkFlags TranslateShaderStage(SpvExecutionModel type);

    SpvExecutionModel GetShaderType()
    {
        return this->module.spirv_execution_model;
    }

private:
    SpvReflectShaderModule module = {};
    std::vector<char> shaderCode;
};
