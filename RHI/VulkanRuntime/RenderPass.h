#pragma once

#include <RHI/RenderPass.h>

#include <vulkan/vulkan.h>

class VulkanRenderPass : public RenderPass
{
public:
    VulkanRenderPass(IntrusivePtr<Graph> graph) : RenderPass(graph)
    {
    }

    VkFormat TranslateFormat(Format format)
    {
        return VK_FORMAT_B8G8R8A8_UNORM;
    }

    void Build()
    {
        this->graph->Topo();

        for (auto &[level, subPassNode] : this->graph->graphNodesMap)
        {
            if (subPassNode->type != GraphNode::GRAPHIC_PASS)
            {
                continue;
            }

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

            std::vector<VkAttachmentDescription> attachments(typedNodes.attachmentNodes.size());
            std::unordered_map<std::string, uint32_t> attachmentMap;

            // find all attachment
            for (int i = 0; i < typedNodes.attachmentNodes.size(); i++)
            {
                auto attachmentNode = typedNodes.attachmentNodes[i];

                if (attachmentNode->depthStencil)
                {
                    attachments[i].format = TranslateFormat(attachmentNode->format);
                    attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
                    attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                else
                {
                    attachments[i].format = TranslateFormat(attachmentNode->format);
                    attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
                    attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                    attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }

                attachmentMap[attachmentNode->name] = i;
            }


            std::vector<VkAttachmentReference> colorRefs;
            std::vector<VkAttachmentReference> depthRef;
            std::vector<VkAttachmentReference> inputRefs;
            for (auto output : subPassNode->outputs) {
                if (output->type == GraphNode::ATTACHMENT) {
                    auto attachentInput = static_cast<AttachmentGraphNode*>(output.get());
                    if (attachentInput->depthStencil) {
                        depthRef.push_back({attachmentMap[output->name], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
                    }
                    else {
                        colorRefs.push_back({attachmentMap[output->name], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                    }
                }
            }
             
            for (auto input : subPassNode->inputs) {
                if (input->type == GraphNode::ATTACHMENT) {
                    inputRefs.push_back({attachmentMap[input->name], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                }
            }

            VkSubpassDescription subPassDescription = {};
            subPassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subPassDescription.inputAttachmentCount = (uint32_t)inputRefs.size();
            subPassDescription.pInputAttachments = inputRefs.data();
            subPassDescription.colorAttachmentCount = (uint32_t)colorRefs.size();
            subPassDescription.pColorAttachments = colorRefs.data();
            subPassDescription.pDepthStencilAttachment = depthRef.data();

            VkRenderPassCreateInfo renderPassInfoCI = {};
            renderPassInfoCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfoCI.attachmentCount = (uint32_t)(attachments.size());
            renderPassInfoCI.pAttachments = attachments.data();
            renderPassInfoCI.subpassCount = 1;
            renderPassInfoCI.pSubpasses = &subPassDescription;
            renderPassInfoCI.dependencyCount = 0;
            renderPassInfoCI.pDependencies = nullptr;
        }
    }
};