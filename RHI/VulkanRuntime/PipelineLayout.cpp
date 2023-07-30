#include <RHI/VulkanRuntime/PipelineLayout.h>

#include <spirv_reflect.h>

#include <spdlog/spdlog.h>

VkDescriptorType TranslateReflectDescriptorType(SpvReflectDescriptorType type)
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

VkShaderStageFlagBits TranslateShaderStage(SpvExecutionModel type)
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

void VulkanPipelineLayout::Build()
{
    for (auto &v : descriptorSets)
    {
        VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
        descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayout.pNext = nullptr;
        descriptorLayout.bindingCount = (uint32_t)v.size();
        descriptorLayout.pBindings = v.data();

        VkDescriptorSetLayout layout;
        auto result = vkCreateDescriptorSetLayout(context->GetVkDevice(), &descriptorLayout, nullptr, &layout);
        descriptorSetLayouts.push_back(layout);
    }

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    pPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

    auto result = vkCreatePipelineLayout(context->GetVkDevice(), &pPipelineLayoutCreateInfo, nullptr, &this->pipelineLayout);
}

void VulkanPipelineLayout::ParseFromReflect(std::vector<char> spirv_code)
{
    // Generate reflection data for a shader
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(spirv_code.size(), spirv_code.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    auto shaderType = module.entry_points[0].spirv_execution_model;

    // Enumerate and extract shader's input variables
    uint32_t var_count = 0;
    result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    SpvReflectInterfaceVariable **input_vars =
        (SpvReflectInterfaceVariable **)malloc(var_count * sizeof(SpvReflectInterfaceVariable *));
    result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    // descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    // descriptorLayout.pNext = nullptr;
    // descriptorLayout.bindingCount = 1;
    // descriptorLayout.pBindings = &layoutBinding;

    {
        spvReflectEnumerateDescriptorSets(&module, &var_count, NULL);
        SpvReflectDescriptorSet **reflectDescriptorSets =
            (SpvReflectDescriptorSet **)malloc(var_count * sizeof(SpvReflectDescriptorSet *));
        result = spvReflectEnumerateDescriptorSets(&module, &var_count, reflectDescriptorSets);

        for (int i = 0; i < var_count; i++)
        {
            auto reflectDescriptorSet = reflectDescriptorSets[i];
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            for (int j = 0; j < reflectDescriptorSet->binding_count; j++)
            {
                auto reflectBinding = reflectDescriptorSets[i]->bindings[j];
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.descriptorType = TranslateReflectDescriptorType(reflectBinding->descriptor_type);
                layoutBinding.descriptorCount = reflectBinding->count;
                layoutBinding.stageFlags = TranslateShaderStage(shaderType);
                layoutBinding.pImmutableSamplers = nullptr;

                if (shaderType == SpvExecutionModelVertex)
                {
                    vertexShaderDescriptorBindingMap[reflectBinding->name] = layoutBinding;
                }
                else if (shaderType == SpvExecutionModelFragment)
                {
                    fragmentShaderDescriptorBindingMap[reflectBinding->name] = layoutBinding;
                }
                bindings.push_back(layoutBinding);
            }

            descriptorSets.push_back(bindings);
        }
    }

    spdlog::info("reflecting inputs size: {}", var_count);
    for (int i = 0; i < var_count; i++)
    {
        auto input_var = input_vars[i];
        spdlog::info("name: {}", input_var->name);
    }

    // Output variables, descriptor bindings, descriptor sets, and push constants
    // can be enumerated and extracted using a similar mechanism.

    // Destroy the reflection data when no longer required.
    spvReflectDestroyShaderModule(&module);
}
