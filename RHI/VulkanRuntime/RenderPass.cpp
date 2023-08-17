#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/Texture.h>

#include <iterator>

VulkanRenderPass::VulkanRenderPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph) : RenderPass(graph), context(context)
{
}

VulkanRenderPass::~VulkanRenderPass()
{
    vkDestroyRenderPass(context->GetVkDevice(), renderPass, nullptr);
}

void VulkanRenderPass::Build()
{
    // todo: handle multisubpassed
    std::vector<VkAttachmentDescription> attachmentsDescriptions;
    std::vector<VkSubpassDescription> subPassDescriptions;
    uint32_t attachmentCounter = 0;
    for (auto &[level, subPassNode] : this->graph->graphNodesMap)
    {
        if (subPassNode->type != GraphNode::GRAPHIC_PASS)
        {
            continue;
        }

        std::vector<IntrusivePtr<AttachmentGraphNode>> subPassAttachmentNodes;

        graphicRenderPasses.push_back(static_cast<GraphicRenderPassGraphNode *>(subPassNode.get()));

        auto &referencesGroup = this->referencesMap[subPassNode->name];

        for (auto n : subPassNode->inputs)
        {
            if (n->type == GraphNode::ATTACHMENT)
            {
                auto agn = static_cast<AttachmentGraphNode *>(n.get());
                agn->input = true;
                subPassAttachmentNodes.push_back(agn);
                referencesGroup.depthRef.push_back({attachmentCounter++, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
            }
        }

        for (auto output : subPassNode->outputs)
        {
            if (output->type == GraphNode::ATTACHMENT)
            {
                auto agn = static_cast<AttachmentGraphNode *>(output.get());
                if (agn->depthStencil)
                {
                    referencesGroup.depthRef.push_back({attachmentCounter++, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
                }
                else
                {
                    referencesGroup.colorRefs.push_back({attachmentCounter++, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                }
                subPassAttachmentNodes.push_back(agn);
            }
        }

        // find all attachment
        for (int i = 0; i < subPassAttachmentNodes.size(); i++)
        {
            auto attachmentNode = subPassAttachmentNodes[i];
            VkAttachmentDescription desc = {};

            if (attachmentNode->depthStencil)
            {
                desc.format = GeneralFormatToVkFormat(attachmentNode->format);
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            else
            {
                desc.format = GeneralFormatToVkFormat(attachmentNode->format);
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            attachmentsDescriptions.push_back(desc);
        }

        attachmentNodes.insert(std::end(attachmentNodes), subPassAttachmentNodes.begin(), subPassAttachmentNodes.end());

        VkSubpassDescription subPassDescription = {};
        subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPassDescription.inputAttachmentCount = (uint32_t)referencesGroup.inputRefs.size();
        subPassDescription.pInputAttachments = referencesGroup.inputRefs.data();
        subPassDescription.colorAttachmentCount = (uint32_t)referencesGroup.colorRefs.size();
        subPassDescription.pColorAttachments = referencesGroup.colorRefs.data();
        subPassDescription.pDepthStencilAttachment = referencesGroup.depthRef.data();
        subPassDescriptions.push_back(subPassDescription);
    }

    VkRenderPassCreateInfo renderPassInfoCI = {};
    renderPassInfoCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfoCI.attachmentCount = (uint32_t)(attachmentsDescriptions.size());
    renderPassInfoCI.pAttachments = attachmentsDescriptions.data();
    renderPassInfoCI.subpassCount = subPassDescriptions.size();
    renderPassInfoCI.pSubpasses = subPassDescriptions.data();
    renderPassInfoCI.dependencyCount = 0;
    renderPassInfoCI.pDependencies = nullptr;

    auto result = vkCreateRenderPass(context->GetVkDevice(), &renderPassInfoCI, nullptr, &this->renderPass);
}

void VulkanRenderPass::RegisterPipeline(std::string name, IntrusivePtr<VulkanPipeline> pipeline)
{
    pipelineMap[name] = pipeline;
}
