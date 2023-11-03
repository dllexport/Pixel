#include <RHI/VulkanRuntime/Pipeline.h>

VulkanPipeline::VulkanPipeline(IntrusivePtr<Context> context, std::string groupName, std::string pipelineName) : Pipeline(groupName, pipelineName), context(context)
{
}

VkShaderModule VulkanPipeline::loadShader(std::string path, VkShaderStageFlagBits stage)
{
    auto shaderCode = ReadBinaryFile(path);

    if (!shaderCode.empty())
    {
        VkShaderModule shaderModule;
        VkShaderModuleCreateInfo moduleCreateInfo = {};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.codeSize = (uint32_t)shaderCode.size();
        moduleCreateInfo.pCode = (uint32_t *)shaderCode.data();

        auto result = vkCreateShaderModule(context->GetVkDevice(), &moduleCreateInfo, NULL, &shaderModule);

        this->shaderCode[stage] = shaderCode;

        return shaderModule;
    }
    else
    {
        return VK_NULL_HANDLE;
    }
}

std::vector<VkPipelineShaderStageCreateInfo> VulkanPipeline::TranslateShaderState(ShaderState state)
{
    std::vector<VkPipelineShaderStageCreateInfo> result;

    if (!state.vertexShaderPath.empty())
    {
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStage.module = loadShader(state.vertexShaderPath, VK_SHADER_STAGE_VERTEX_BIT);
        shaderStage.pName = "main";
        assert(shaderStage.module != VK_NULL_HANDLE);
        shaderModules.push_back(shaderStage.module);
        result.push_back(shaderStage);
    }

    if (!state.fragmentShaderPath.empty())
    {
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStage.module = loadShader(state.fragmentShaderPath, VK_SHADER_STAGE_FRAGMENT_BIT);
        shaderStage.pName = "main";
        assert(shaderStage.module != VK_NULL_HANDLE);
        shaderModules.push_back(shaderStage.module);
        result.push_back(shaderStage);
    }

    if (!state.computeShaderPath.empty())
    {
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStage.module = loadShader(state.computeShaderPath, VK_SHADER_STAGE_COMPUTE_BIT);
        shaderStage.pName = "main";
        assert(shaderStage.module != VK_NULL_HANDLE);
        shaderModules.push_back(shaderStage.module);
        result.push_back(shaderStage);
    }

    return result;
}