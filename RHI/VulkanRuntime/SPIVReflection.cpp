#include <RHI/VulkanRuntime/SPIVReflection.h>

#include <spdlog/spdlog.h>

VkFormat TranslateReflectFormat(SpvReflectFormat format)
{
    switch (format)
    {
    case SPV_REFLECT_FORMAT_UNDEFINED:
        return VK_FORMAT_UNDEFINED;
    case SPV_REFLECT_FORMAT_R16_UINT:
        return VK_FORMAT_R16_UINT;
    case SPV_REFLECT_FORMAT_R16_SINT:
        return VK_FORMAT_R16_SINT;
    case SPV_REFLECT_FORMAT_R16_SFLOAT:
        return VK_FORMAT_R16_SFLOAT;
    case SPV_REFLECT_FORMAT_R16G16_UINT:
        return VK_FORMAT_R16G16_UINT;
    case SPV_REFLECT_FORMAT_R16G16_SINT:
        return VK_FORMAT_R16G16_SINT;
    case SPV_REFLECT_FORMAT_R16G16_SFLOAT:
        return VK_FORMAT_R16G16_SFLOAT;
    case SPV_REFLECT_FORMAT_R16G16B16_UINT:
        return VK_FORMAT_R16G16B16_UINT;
    case SPV_REFLECT_FORMAT_R16G16B16_SINT:
        return VK_FORMAT_R16G16B16_SINT;
    case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:
        return VK_FORMAT_R16G16B16_SFLOAT;
    case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
        return VK_FORMAT_R16G16B16A16_UINT;
    case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
        return VK_FORMAT_R16G16B16A16_SINT;
    case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case SPV_REFLECT_FORMAT_R32_UINT:
        return VK_FORMAT_R32_UINT;
    case SPV_REFLECT_FORMAT_R32_SINT:
        return VK_FORMAT_R32_SINT;
    case SPV_REFLECT_FORMAT_R32_SFLOAT:
        return VK_FORMAT_R32_SFLOAT;
    case SPV_REFLECT_FORMAT_R32G32_UINT:
        return VK_FORMAT_R32G32_UINT;
    case SPV_REFLECT_FORMAT_R32G32_SINT:
        return VK_FORMAT_R32G32_SINT;
    case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
        return VK_FORMAT_R32G32_SFLOAT;
    case SPV_REFLECT_FORMAT_R32G32B32_UINT:
        return VK_FORMAT_R32G32B32_UINT;
    case SPV_REFLECT_FORMAT_R32G32B32_SINT:
        return VK_FORMAT_R32G32B32_SINT;
    case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
        return VK_FORMAT_R32G32B32A32_UINT;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
        return VK_FORMAT_R32G32B32A32_SINT;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case SPV_REFLECT_FORMAT_R64_UINT:
        return VK_FORMAT_R64_UINT;
    case SPV_REFLECT_FORMAT_R64_SINT:
        return VK_FORMAT_R64_SINT;
    case SPV_REFLECT_FORMAT_R64_SFLOAT:
        return VK_FORMAT_R64_SFLOAT;
    case SPV_REFLECT_FORMAT_R64G64_UINT:
        return VK_FORMAT_R64G64_UINT;
    case SPV_REFLECT_FORMAT_R64G64_SINT:
        return VK_FORMAT_R64G64_SINT;
    case SPV_REFLECT_FORMAT_R64G64_SFLOAT:
        return VK_FORMAT_R64G64_SFLOAT;
    case SPV_REFLECT_FORMAT_R64G64B64_UINT:
        return VK_FORMAT_R64G64B64_UINT;
    case SPV_REFLECT_FORMAT_R64G64B64_SINT:
        return VK_FORMAT_R64G64B64_SINT;
    case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:
        return VK_FORMAT_R64G64B64_SFLOAT;
    case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:
        return VK_FORMAT_R64G64B64A64_UINT;
    case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:
        return VK_FORMAT_R64G64B64A64_SINT;
    case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:
        return VK_FORMAT_R64G64B64A64_SFLOAT;
    }
};

uint32_t TranslateReflectFormatToSize(SpvReflectFormat format)
{
    switch (format)
    {
    case SPV_REFLECT_FORMAT_UNDEFINED:
        return 0;
    case SPV_REFLECT_FORMAT_R16_UINT:
        return 2;
    case SPV_REFLECT_FORMAT_R16_SINT:
        return 2;
    case SPV_REFLECT_FORMAT_R16_SFLOAT:
        return 2;
    case SPV_REFLECT_FORMAT_R16G16_UINT:
        return 4;
    case SPV_REFLECT_FORMAT_R16G16_SINT:
        return 4;
    case SPV_REFLECT_FORMAT_R16G16_SFLOAT:
        return 4;
    case SPV_REFLECT_FORMAT_R16G16B16_UINT:
        return 6;
    case SPV_REFLECT_FORMAT_R16G16B16_SINT:
        return 6;
    case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:
        return 6;
    case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
        return 8;
    case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
        return 8;
    case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:
        return 8;
    case SPV_REFLECT_FORMAT_R32_UINT:
        return 4;
    case SPV_REFLECT_FORMAT_R32_SINT:
        return 4;
    case SPV_REFLECT_FORMAT_R32_SFLOAT:
        return 4;
    case SPV_REFLECT_FORMAT_R32G32_UINT:
        return 8;
    case SPV_REFLECT_FORMAT_R32G32_SINT:
        return 8;
    case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
        return 8;
    case SPV_REFLECT_FORMAT_R32G32B32_UINT:
        return 12;
    case SPV_REFLECT_FORMAT_R32G32B32_SINT:
        return 12;
    case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
        return 12;
    case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
        return 16;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
        return 16;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
        return 16;
    case SPV_REFLECT_FORMAT_R64_UINT:
        return 8;
    case SPV_REFLECT_FORMAT_R64_SINT:
        return 8;
    case SPV_REFLECT_FORMAT_R64_SFLOAT:
        return 8;
    case SPV_REFLECT_FORMAT_R64G64_UINT:
        return 16;
    case SPV_REFLECT_FORMAT_R64G64_SINT:
        return 16;
    case SPV_REFLECT_FORMAT_R64G64_SFLOAT:
        return 16;
    case SPV_REFLECT_FORMAT_R64G64B64_UINT:
        return 24;
    case SPV_REFLECT_FORMAT_R64G64B64_SINT:
        return 24;
    case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:
        return 24;
    case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:
        return 32;
    case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:
        return 32;
    case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:
        return 32;
    }
};

SPIVReflection::SPIVReflection(std::vector<char> shaderCode) : shaderCode(shaderCode)
{
    // Generate reflection data for a shader
    SpvReflectResult result = spvReflectCreateShaderModule(shaderCode.size(), shaderCode.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
}

SPIVReflection::~SPIVReflection()
{
    // Destroy the reflection data when no longer required.
    spvReflectDestroyShaderModule(&module);
}

SPIVReflection::InputVertexState SPIVReflection::ParseInputVertexState()
{
    auto shaderType = module.entry_points[0].spirv_execution_model;

    // Enumerate and extract shader's input variables
    uint32_t inputVarSize = 0;
    auto result = spvReflectEnumerateInputVariables(&module, &inputVarSize, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<SpvReflectInterfaceVariable *> inputVars(inputVarSize);
    result = spvReflectEnumerateInputVariables(&module, &inputVarSize, inputVars.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // TODO, handle multi binding case

    std::sort(inputVars.begin(), inputVars.end(), [](SpvReflectInterfaceVariable *left, SpvReflectInterfaceVariable *right)
              { return left->location < right->location; });

    std::vector<VkVertexInputAttributeDescription> inputAttributes;
    spdlog::info("reflecting inputs size: {}", inputVars.size());
    uint32_t offset = 0;
    for (auto inputVar : inputVars)
    {
        inputAttributes.push_back(VkVertexInputAttributeDescription{
            .location = inputVar->location,
            .binding = 0,
            .format = TranslateReflectFormat(inputVar->format),
            .offset = offset});

        offset += TranslateReflectFormatToSize(inputVar->format);
    }
    std::vector<VkVertexInputBindingDescription> inputBindings{{0, offset, VK_VERTEX_INPUT_RATE_VERTEX}};

    return {inputBindings, inputAttributes};
}

SPIVReflection::DescriptorLayoutState SPIVReflection::ParseDescriptorLayoutState()
{
    SPIVReflection::DescriptorLayoutState dls = {};

    uint32_t setCount;
    spvReflectEnumerateDescriptorSets(&module, &setCount, NULL);
    std::vector<SpvReflectDescriptorSet *> reflectDescriptorSets(setCount);
    auto result = spvReflectEnumerateDescriptorSets(&module, &setCount, reflectDescriptorSets.data());

    std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorLayoutState;

    for (int i = 0; i < setCount; i++)
    {
        auto reflectDescriptorSet = reflectDescriptorSets[i];
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        for (int j = 0; j < reflectDescriptorSet->binding_count; j++)
        {
            auto reflectBinding = reflectDescriptorSets[i]->bindings[j];
            VkDescriptorSetLayoutBinding layoutBinding = {};
            layoutBinding.binding = reflectBinding->binding;
            layoutBinding.descriptorType = TranslateReflectDescriptorType(reflectBinding->descriptor_type);
            layoutBinding.descriptorCount = reflectBinding->count;
            layoutBinding.stageFlags = TranslateShaderStage(module.spirv_execution_model);
            layoutBinding.pImmutableSamplers = nullptr;
            bindings.push_back(layoutBinding);
        }

        descriptorLayoutState.push_back(bindings);
    }

    uint32_t pushConstantCount;
    spvReflectEnumeratePushConstants(&module, &pushConstantCount, NULL);
    assert(pushConstantCount <= 1);
    std::vector<SpvReflectBlockVariable *> reflectDescriptorPushConstant(pushConstantCount);
    result = spvReflectEnumeratePushConstants(&module, &pushConstantCount, reflectDescriptorPushConstant.data());
    if (!reflectDescriptorPushConstant.empty())
    {
        VkPushConstantRange range = {
            .stageFlags = TranslateShaderStage(module.spirv_execution_model),
            .offset = reflectDescriptorPushConstant[0]->offset,
            .size = reflectDescriptorPushConstant[0]->size};
        dls.pushConstantRange = range;
    }

    dls.descriptorSetLayoutSets = std::move(descriptorLayoutState);
    return dls;
}

VkDescriptorType SPIVReflection::TranslateReflectDescriptorType(SpvReflectDescriptorType type)
{
    switch (type)
    {
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    }

    return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

VkFlags SPIVReflection::TranslateShaderStage(SpvExecutionModel type)
{
    switch (type)
    {
    case SpvExecutionModelVertex:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case SpvExecutionModelFragment:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    default:
    {
        return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }
    }
}