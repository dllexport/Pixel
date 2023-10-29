#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/Texture.h>

#include <Core/ReadFile.h>

VkPipelineInputAssemblyStateCreateInfo TranslateInputAssemblyState(InputAssembleState state)
{
    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
    pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    VkPrimitiveTopology topology;
    switch (state.type)
    {
    case InputAssembleState::Type::POINT:
    {
        topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        break;
    }
    case InputAssembleState::Type::LINE_LIST:
    {
        topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        break;
    }
    case InputAssembleState::Type::LINE_STRIP:
    {
        topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;

        break;
    }
    case InputAssembleState::Type::TRIANGLE_LIST:
    {
        topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        break;
    }
    case InputAssembleState::Type::TRIANGLE_STRIP:
    {
        topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        break;
    }
    case InputAssembleState::Type::TRIANGLE_FAN:
    {
        topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

        break;
    }
    }

    pipelineInputAssemblyStateCreateInfo.topology = topology;
    pipelineInputAssemblyStateCreateInfo.flags = {};
    pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = false;
    return pipelineInputAssemblyStateCreateInfo;
}

VkPipelineRasterizationStateCreateInfo TranslateRasterizationState(RasterizationState state)
{
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    VkPolygonMode polygonMode;
    switch (state.polygonMode)
    {
    case RasterizationState::PolygonModeType::FILL:
    {
        polygonMode = VK_POLYGON_MODE_FILL;
        break;
    }
    case RasterizationState::PolygonModeType::LINE:
    {
        polygonMode = VK_POLYGON_MODE_LINE;
        break;
    }
    case RasterizationState::PolygonModeType::POINT:
    {
        polygonMode = VK_POLYGON_MODE_POINT;
        break;
    }
    }
    pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;

    VkCullModeFlags cullMode;
    switch (state.cullMode)
    {
    case RasterizationState::CullModeType::NONE:
    {
        cullMode = VK_CULL_MODE_NONE;
        break;
    }
    case RasterizationState::CullModeType::FRONT:
    {
        cullMode = VK_CULL_MODE_FRONT_BIT;
        break;
    }
    case RasterizationState::CullModeType::BACK:
    {
        cullMode = VK_CULL_MODE_BACK_BIT;
        break;
    }
    case RasterizationState::CullModeType::FRONT_AND_BACK:
    {
        cullMode = VK_CULL_MODE_FRONT_AND_BACK;
        break;
    }
    }
    pipelineRasterizationStateCreateInfo.cullMode = cullMode;

    VkFrontFace frontFace;
    switch (state.frontFace)
    {
    case RasterizationState::FrontFaceType::COUNTER_CLOCKWISE:
    {
        frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        break;
    }
    case RasterizationState::FrontFaceType::CLOCKWISE:
    {
        frontFace = VK_FRONT_FACE_CLOCKWISE;
        break;
    }
    }
    pipelineRasterizationStateCreateInfo.frontFace = frontFace;

    pipelineRasterizationStateCreateInfo.flags = {};
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.lineWidth = state.lineWidth;
    return pipelineRasterizationStateCreateInfo;
}

VkPipelineDepthStencilStateCreateInfo TranslateDepthStencilState(DepthStencilState state)
{
    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
    pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    pipelineDepthStencilStateCreateInfo.depthTestEnable = state.depthTestEnable;
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = state.depthWriteEnable;
    pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    return pipelineDepthStencilStateCreateInfo;
}

VkPipelineViewportStateCreateInfo TranslateViewportState()
{
    VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
    pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportStateCreateInfo.viewportCount = 1;
    pipelineViewportStateCreateInfo.scissorCount = 1;
    pipelineViewportStateCreateInfo.flags = {};
    return pipelineViewportStateCreateInfo;
}

VkPipelineMultisampleStateCreateInfo TranslateMultisampleState()
{
    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
    pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineMultisampleStateCreateInfo.flags = {};
    return pipelineMultisampleStateCreateInfo;
}

std::vector<VkPipelineColorBlendAttachmentState> TranslateColorBlendAttachmentState(std::vector<ColorBlendAttachmentState> states, uint32_t subPassColorAttachmentSize)
{
    std::vector<VkPipelineColorBlendAttachmentState> result;

    if (states.empty())
    {
        for (int i = 0; i < subPassColorAttachmentSize; i++)
        {
            result.push_back(VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .colorWriteMask = 0xf});
        }
    }

    for (auto state : states)
    {
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
        pipelineColorBlendAttachmentState.blendEnable = (VkBool32)state.blendEnable;
        pipelineColorBlendAttachmentState.srcColorBlendFactor = (VkBlendFactor)state.srcColorBlendFactor;
        pipelineColorBlendAttachmentState.dstColorBlendFactor = (VkBlendFactor)state.dstColorBlendFactor;
        pipelineColorBlendAttachmentState.colorBlendOp = (VkBlendOp)state.colorBlendOp;
        pipelineColorBlendAttachmentState.srcAlphaBlendFactor = (VkBlendFactor)state.srcAlphaBlendFactor;
        pipelineColorBlendAttachmentState.dstAlphaBlendFactor = (VkBlendFactor)state.dstAlphaBlendFactor;
        pipelineColorBlendAttachmentState.alphaBlendOp = (VkBlendOp)state.alphaBlendOp;
        pipelineColorBlendAttachmentState.colorWriteMask = (VkColorComponentFlags)state.colorWriteMask;
        result.push_back(pipelineColorBlendAttachmentState);
    }

    assert(result.size() == subPassColorAttachmentSize);

    return result;
}

VkPipelineColorBlendStateCreateInfo BuildColorBlendAttachmentState(std::vector<VkPipelineColorBlendAttachmentState> &pAttachments)
{
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.attachmentCount = (uint32_t)pAttachments.size();
    pipelineColorBlendStateCreateInfo.pAttachments = pAttachments.data();
    return pipelineColorBlendStateCreateInfo;
}

VkPipelineDynamicStateCreateInfo TranslateDynamicState(std::vector<VkDynamicState> &dynamicStateEnables)
{
    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
    pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    pipelineDynamicStateCreateInfo.flags = {};
    return pipelineDynamicStateCreateInfo;
}

VkPipeline VulkanGraphicsPipeline::GetPipeline()
{
    return pipeline;
}

IntrusivePtr<VulkanRenderPass> VulkanGraphicsPipeline::GetRenderPass()
{
    return renderPass;
}

VkShaderModule VulkanGraphicsPipeline::loadShader(std::string path, VkShaderStageFlagBits stage)
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

std::vector<VkPipelineShaderStageCreateInfo> VulkanGraphicsPipeline::TranslateShaderState(ShaderState state)
{
    std::vector<VkPipelineShaderStageCreateInfo> result;
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

    return result;
}

VkPipelineVertexInputStateCreateInfo TranslateInputVertexState(SPIVReflection::InputVertexState &state)
{
    VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {};
    vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCI.vertexBindingDescriptionCount = (uint32_t)state.inputBindingDescriptions.size();
    vertexInputStateCI.pVertexBindingDescriptions = state.inputBindingDescriptions.data();
    vertexInputStateCI.vertexAttributeDescriptionCount = (uint32_t)state.inputAttributeDescriptions.size();
    vertexInputStateCI.pVertexAttributeDescriptions = state.inputAttributeDescriptions.data();
    return vertexInputStateCI;
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(IntrusivePtr<Context> context, IntrusivePtr<VulkanRenderPass> renderPass, std::string name, PipelineStates pipelineStates) : Pipeline(), context(context), pipelineStates(pipelineStates)
{
    this->name = name;
    this->renderPass = renderPass;
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
    auto device = context->GetVkDevice();
    for (auto &shaderModule : shaderModules)
    {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }

    vkDestroyPipeline(device, pipeline, nullptr);
}

IntrusivePtr<VulkanPipelineLayout> &VulkanGraphicsPipeline::GetPipelineLayout()
{
    return pipelineLayout;
}

void VulkanGraphicsPipeline::Build()
{
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = TranslateInputAssemblyState(pipelineStates.inputAssembleState);
    VkPipelineRasterizationStateCreateInfo rasterizationStateCI = TranslateRasterizationState(pipelineStates.rasterizationState);

    // apply default blending state
    auto vulkanRenderPass = static_cast<VulkanRenderPass *>(renderPass.get());
    auto colorRefsSize = vulkanRenderPass->attachmentReferencesMap[this->name].colorRefs.size();
    auto colorBlendAttachmentStates = TranslateColorBlendAttachmentState(pipelineStates.colorBlendAttachmentStates, colorRefsSize);
    VkPipelineColorBlendStateCreateInfo colorBlendStateCI = BuildColorBlendAttachmentState(colorBlendAttachmentStates);
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = TranslateDepthStencilState(pipelineStates.depthStencilState);
    VkPipelineViewportStateCreateInfo viewportStateCI = TranslateViewportState();
    VkPipelineMultisampleStateCreateInfo multisampleStateCI = TranslateMultisampleState();

    std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCI = TranslateDynamicState(dynamicStateEnables);

    auto subPassIndex = vulkanRenderPass->GetSubPassIndex(this->name);

    if (pipelineStates.shaderState.Empty())
    {
        auto grp = vulkanRenderPass->graphicRenderPasses[subPassIndex];
        pipelineStates.shaderState.vertexShaderPath = grp->vertexShader;
        pipelineStates.shaderState.fragmentShaderPath = grp->framgmentShader;
    }
    std::vector<VkPipelineShaderStageCreateInfo> shaderStateCI = TranslateShaderState(pipelineStates.shaderState);

    VkGraphicsPipelineCreateInfo pipelineCI = {};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.basePipelineIndex = -1;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCI.renderPass = vulkanRenderPass->GetRenderPass();
    pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
    pipelineCI.pRasterizationState = &rasterizationStateCI;
    pipelineCI.pColorBlendState = &colorBlendStateCI;
    pipelineCI.pMultisampleState = &multisampleStateCI;
    pipelineCI.pViewportState = &viewportStateCI;
    pipelineCI.pDepthStencilState = &depthStencilStateCI;
    pipelineCI.pDynamicState = &dynamicStateCI;
    pipelineCI.stageCount = static_cast<uint32_t>(shaderStateCI.size());
    pipelineCI.pStages = shaderStateCI.data();
    pipelineCI.subpass = subPassIndex;

    IntrusivePtr<SPIVReflection> vertexReflection = new SPIVReflection(shaderCode[VK_SHADER_STAGE_VERTEX_BIT]);

    SPIVReflection::InputVertexState inputVertexState;

    if (pipelineStates.vertexInputStates.empty())
    {
        inputVertexState = vertexReflection->ParseInputVertexState();
    }
    else
    {
        // reuse InputVertexState
        uint32_t offset = 0;
        for (auto vis : pipelineStates.vertexInputStates)
        {
            VkVertexInputAttributeDescription viad = {
                .location = vis.location,
                .binding = vis.binding,
                .format = GeneralFormatToVkFormat(vis.format),
                .offset = offset};
            offset += GeneralFormatToSize(vis.format);
            inputVertexState.inputAttributeDescriptions.push_back(viad);
        }
        VkVertexInputBindingDescription ibd = {
            .binding = 0,
            .stride = offset,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
        inputVertexState.inputBindingDescriptions.push_back(ibd);
    }

    auto inputVertexStateCI = TranslateInputVertexState(inputVertexState);
    pipelineCI.pVertexInputState = &inputVertexStateCI;
    IntrusivePtr<SPIVReflection> fragmentReflection = new SPIVReflection(shaderCode[VK_SHADER_STAGE_FRAGMENT_BIT]);
    this->pipelineLayout = new VulkanPipelineLayout(context);
    this->pipelineLayout->Build({vertexReflection, fragmentReflection});
    pipelineCI.layout = this->pipelineLayout->GetLayout();

    auto result = vkCreateGraphicsPipelines(context->GetVkDevice(), nullptr, 1, &pipelineCI, nullptr, &pipeline);
}
