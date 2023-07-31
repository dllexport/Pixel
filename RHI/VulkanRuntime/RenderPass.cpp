#include <RHI/VulkanRuntime/RenderPass.h>

#include <RHI/VulkanRuntime/Runtime.h>

VulkanRenderPass::VulkanRenderPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph) : RenderPass(graph), context(context)
{
}

VulkanRenderPass::~VulkanRenderPass()
{
    vkDestroyRenderPass(context->GetVkDevice(), renderPass, nullptr);
}

VkFormat TranslateFormat(TextureFormat format, bool isDepthStencil)
{
    if (isDepthStencil)
    {
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
    return VK_FORMAT_B8G8R8A8_UNORM;
}

void VulkanRenderPass::Build()
{
    this->graph->Topo();

    // TODO, aggregate subpasses
    for (auto &[level, subPassNode] : this->graph->graphNodesMap)
    {
        if (subPassNode->type != GraphNode::GRAPHIC_PASS)
        {
            continue;
        }

        graphicRenderPasses.push_back(static_cast<GraphicRenderPassGraphNode *>(subPassNode.get()));

        struct
        {
            std::vector<IntrusivePtr<AttachmentGraphNode>> attachmentNodes;
            std::vector<IntrusivePtr<DescriptorGraphNode>> descriptorNodes;
        } typedNodes;

        for (auto n : subPassNode->inputs)
        {
            if (n->type == GraphNode::ATTACHMENT)
            {
                typedNodes.attachmentNodes.push_back(static_cast<AttachmentGraphNode *>(n.get()));
            }
            else if (n->type == GraphNode::BUFFER)
            {
                typedNodes.descriptorNodes.push_back(static_cast<DescriptorGraphNode *>(n.get()));
            }
        }

        for (auto n : subPassNode->outputs)
        {
            if (n->type == GraphNode::ATTACHMENT)
            {
                typedNodes.attachmentNodes.push_back(static_cast<AttachmentGraphNode *>(n.get()));
            }
            else if (n->type == GraphNode::BUFFER)
            {
                typedNodes.descriptorNodes.push_back(static_cast<DescriptorGraphNode *>(n.get()));
            }
        }

        attachments.resize(typedNodes.attachmentNodes.size());
        std::unordered_map<std::string, uint32_t> attachmentMap;

        // find all attachment
        for (int i = 0; i < typedNodes.attachmentNodes.size(); i++)
        {
            auto attachmentNode = typedNodes.attachmentNodes[i];

            if (attachmentNode->depthStencil)
            {
                attachments[i].format = TranslateFormat(attachmentNode->format, true);
                attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
                attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            else
            {
                attachments[i].format = TranslateFormat(attachmentNode->format, false);
                attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
                attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }

            attachmentMap[attachmentNode->name] = i;
        }

        auto &referencesGroup = this->referencesMap[subPassNode->name];

        for (auto output : subPassNode->outputs)
        {
            if (output->type == GraphNode::ATTACHMENT)
            {
                auto attachentInput = static_cast<AttachmentGraphNode *>(output.get());
                if (attachentInput->depthStencil)
                {
                    referencesGroup.depthRef.push_back({attachmentMap[output->name], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
                }
                else
                {
                    referencesGroup.colorRefs.push_back({attachmentMap[output->name], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                }
            }
        }

        for (auto input : subPassNode->inputs)
        {
            if (input->type == GraphNode::ATTACHMENT)
            {
                referencesGroup.inputRefs.push_back({attachmentMap[input->name], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
            }
        }

        VkSubpassDescription subPassDescription = {};
        subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPassDescription.inputAttachmentCount = (uint32_t)referencesGroup.inputRefs.size();
        subPassDescription.pInputAttachments = referencesGroup.inputRefs.data();
        subPassDescription.colorAttachmentCount = (uint32_t)referencesGroup.colorRefs.size();
        subPassDescription.pColorAttachments = referencesGroup.colorRefs.data();
        subPassDescription.pDepthStencilAttachment = referencesGroup.depthRef.data();

        VkRenderPassCreateInfo renderPassInfoCI = {};
        renderPassInfoCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfoCI.attachmentCount = (uint32_t)(attachments.size());
        renderPassInfoCI.pAttachments = attachments.data();
        renderPassInfoCI.subpassCount = 1;
        renderPassInfoCI.pSubpasses = &subPassDescription;
        renderPassInfoCI.dependencyCount = 0;
        renderPassInfoCI.pDependencies = nullptr;

        auto device = this->context->GetVkDevice();
        auto result = vkCreateRenderPass(device, &renderPassInfoCI, nullptr, &this->renderPass);

        break;
    }
}
