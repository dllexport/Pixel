#include <RHI/VulkanRuntime/ComputePipeline.h>

VulkanComputePipeline::VulkanComputePipeline(IntrusivePtr<Context> context, IntrusivePtr<VulkanComputePass> computePass, std::string groupName, std::string pipelineName, ComputePipelineStates pipelineStates) : VulkanPipeline(context, groupName, pipelineName), computePass(computePass), pipelineStates(pipelineStates)
{
}

VulkanComputePipeline::~VulkanComputePipeline()
{
}

IntrusivePtr<VulkanPipelineLayout> &VulkanComputePipeline::GetPipelineLayout()
{
    return pipelineLayout;
}

void VulkanComputePipeline::Build()
{
    if (pipelineStates.shaderState.Empty())
    {
        auto crp = computePass->GetRenderPassGraphNode().at(0)->As<ComputeRenderPassGraphNode *>();
        pipelineStates.shaderState.computeShaderPath = crp->computeShader;
    }
    std::vector<VkPipelineShaderStageCreateInfo> shaderStateCI = TranslateShaderState(pipelineStates.shaderState);

    IntrusivePtr<SPIVReflection> computeReflection = new SPIVReflection(shaderCode[VK_SHADER_STAGE_COMPUTE_BIT]);

    this->pipelineLayout = new VulkanPipelineLayout(context);
    this->pipelineLayout->Build({computeReflection});

    VkComputePipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipelineCreateInfo.stage = shaderStateCI.at(0);
    pipelineCreateInfo.layout = pipelineLayout->GetLayout();

    auto result = vkCreateComputePipelines(context->GetVkDevice(),
                                           VK_NULL_HANDLE,
                                           1, &pipelineCreateInfo,
                                           VK_NULL_HANDLE,
                                           &this->pipeline);
}