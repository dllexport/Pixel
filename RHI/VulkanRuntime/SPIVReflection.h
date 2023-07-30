#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <spirv_reflect.h>

#include <Core/IntrusivePtr.h>
 
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
        // each set has it's array of VkDescriptorSetLayoutBinding
        std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutSets;
    };
    DescriptorLayoutState ParseDescriptorLayoutState();

    static VkDescriptorType TranslateReflectDescriptorType(SpvReflectDescriptorType type);
    static VkShaderStageFlagBits TranslateShaderStage(SpvExecutionModel type);

private:
    SpvReflectShaderModule module = {};
    std::vector<char> shaderCode;
};