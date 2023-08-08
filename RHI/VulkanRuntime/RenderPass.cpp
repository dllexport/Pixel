#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/Texture.h>

VulkanRenderPass::VulkanRenderPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph) : RenderPass(graph), context(context)
{
}

VulkanRenderPass::~VulkanRenderPass()
{
    vkDestroyRenderPass(context->GetVkDevice(), renderPass, nullptr);
}

void VulkanRenderPass::Build()
{
    std::vector<VkAttachmentDescription> attachments;

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
                auto agn = static_cast<AttachmentGraphNode *>(n.get());
                agn->input = true;
                typedNodes.attachmentNodes.push_back(agn);
            }
            else if (n->type == GraphNode::BUFFER)
            {
                typedNodes.descriptorNodes.push_back(static_cast<DescriptorGraphNode *>(n.get()));
            }
        }

        for (auto n : subPassNode->outputs)
        {
            // depth field is filled during parsing
            if (n->type == GraphNode::ATTACHMENT)
            {
                auto agn = static_cast<AttachmentGraphNode *>(n.get());
                // color and depth is mutual exclusive
                agn->color = true && !agn->depthStencil;
                typedNodes.attachmentNodes.push_back(agn);
            }
            else if (n->type == GraphNode::BUFFER)
            {
                typedNodes.descriptorNodes.push_back(static_cast<DescriptorGraphNode *>(n.get()));
            }
        }

        attachments.resize(typedNodes.attachmentNodes.size());
        attachmentDescriptions.resize(attachments.size());
        std::unordered_map<std::string, uint32_t> attachmentMap;

        // find all attachment
        for (int i = 0; i < typedNodes.attachmentNodes.size(); i++)
        {
            auto attachmentNode = typedNodes.attachmentNodes[i];

            if (attachmentNode->depthStencil)
            {
                attachments[i].format = GeneralFormatToVkFormat(attachmentNode->format);
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
                attachments[i].format = GeneralFormatToVkFormat(attachmentNode->format);
                attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_NONE_EXT;
                attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }

            attachmentDescriptions[i] = attachmentNode;

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

void VulkanRenderPass::RegisterPipeline(std::string name, IntrusivePtr<VulkanPipeline> pipeline)
{
    pipelineMap[name] = pipeline;
}
