#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/Texture.h>

#include <spdlog/spdlog.h>

#include <optional>
#include <iterator>

VulkanRenderPass::VulkanRenderPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph) : context(context), graph(graph)
{
}

VulkanRenderPass::~VulkanRenderPass()
{
    vkDestroyRenderPass(context->GetVkDevice(), renderPass, nullptr);
}

VkRenderPass VulkanRenderPass::GetRenderPass()
{
    return renderPass;
}

int32_t VulkanRenderPass::GetSubPassIndex(std::string subPassName)
{
    for (int32_t i = 0; i < graphicRenderPasses.size(); i++)
    {
        if (graphicRenderPasses[i]->name == subPassName)
        {
            return i;
        }
    }
    return -1;
}

VulkanRenderPass::SubPassAttachmentReferences VulkanRenderPass::GetSubPassReference(std::string name)
{
    if (attachmentReferencesMap.count(name))
    {
        return attachmentReferencesMap[name];
    }
    return {};
}

IntrusivePtr<GraphicRenderPassGraphNode> VulkanRenderPass::GetGraphicRenderPassGraphNode(uint32_t subPassIndex)
{
    return graphicRenderPasses[subPassIndex];
}

IntrusivePtr<GraphicRenderPassGraphNode> VulkanRenderPass::GetGraphicRenderPassGraphNode(std::string subPassName)
{
    return graphicRenderPasses[GetSubPassIndex(subPassName)];
}

std::optional<VkSubpassDependency> BuildDependency(uint32_t fromIndex, uint32_t toIndex, VulkanRenderPass::SubPassAttachmentReferences from, VulkanRenderPass::SubPassAttachmentReferences to)
{
    std::unordered_set<uint32_t> attachmentIndices;
    if (!from.depthRef.empty())
        attachmentIndices.insert(from.depthRef[0].attachment);

    for (auto &ref : from.colorRefs)
    {
        attachmentIndices.insert(ref.attachment);
    }

    bool raw = false;
    bool waw = false;
    bool depthWAW = false;

    // check RAW
    for (auto &ref : to.inputRefs)
    {
        if (attachmentIndices.count(ref.attachment))
        {
            raw = true;
        }
    }

    // check WAW
    for (auto &ref : to.colorRefs)
    {
        if (attachmentIndices.count(ref.attachment))
        {
            waw = true;
        }
    }

    if (!to.depthRef.empty())
    {
        if (attachmentIndices.count(to.depthRef[0].attachment))
        {
            depthWAW = true;
        }
    }

    if (!waw && !raw && !depthWAW)
    {
        return {};
    }

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = fromIndex;
    dependency.dstSubpass = toIndex;
    if (raw)
    {
        dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependency.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
    }

    if (waw)
    {
        dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }

    if (depthWAW)
    {
        dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return dependency;
}

void VulkanRenderPass::Build(std::vector<std::string> subPasses)
{
    // todo: handle multisubpassed
    std::vector<VkAttachmentDescription> attachmentsDescriptions;
    std::vector<VkSubpassDescription> subPassDescriptions;
    uint32_t attachmentCounter = 0;

    for (auto &[level, subPassNodes] : this->graph->Topo().levelsRenderPassOnly)
    {
        // may have multiple nodes in same level
        for (auto &subPassNode : subPassNodes)
        {
            graphicRenderPasses.push_back(static_cast<GraphicRenderPassGraphNode *>(subPassNode));

            // store the newly created attachment node in this pass
            std::vector<IntrusivePtr<AttachmentGraphNode>> subPassAttachmentNodes;

            auto &referencesGroup = this->attachmentReferencesMap[subPassNode->name];

            for (auto n : subPassNode->inputs)
            {
                if (n->type == GraphNode::ATTACHMENT)
                {
                    auto agn = static_cast<AttachmentGraphNode *>(n.get());
                    int attachmentRefIndex = std::find_if(attachmentNodes.begin(), attachmentNodes.end(), [&](IntrusivePtr<AttachmentGraphNode> &node)
                                                          { return node->name == agn->name; }) -
                                             attachmentNodes.begin();
                    if (attachmentRefIndex >= attachmentNodes.size())
                    {
                        attachmentRefIndex = attachmentCounter++;
                        subPassAttachmentNodes.push_back(agn);
                    }
                    referencesGroup.inputRefs.push_back({(uint32_t)attachmentRefIndex, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
                }
            }

            for (auto output : subPassNode->outputs)
            {
                if (output->type == GraphNode::ATTACHMENT)
                {
                    auto agn = static_cast<AttachmentGraphNode *>(output.get());
                    // try to get attachment ref index (if exist)
                    int attachmentRefIndex = std::find(attachmentNodes.begin(), attachmentNodes.end(), agn) - attachmentNodes.begin();
                    if (attachmentRefIndex >= attachmentNodes.size())
                    {
                        attachmentRefIndex = attachmentCounter++;
                        subPassAttachmentNodes.push_back(agn);
                    }
                    if (agn->depthStencil)
                    {
                        referencesGroup.depthRef.push_back({(uint32_t)attachmentRefIndex, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL});
                    }
                    else
                    {
                        referencesGroup.colorRefs.push_back({(uint32_t)attachmentRefIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                    }
                }
            }

            // for new created attachment, create VkAttachmentDescriptions
            for (int i = 0; i < subPassAttachmentNodes.size(); i++)
            {
                auto attachmentNode = subPassAttachmentNodes[i];
                VkAttachmentDescription desc = {};

                if (attachmentNode->depthStencil)
                {
                    desc.format = GeneralFormatToVkFormat(attachmentNode->format);
                    desc.samples = VK_SAMPLE_COUNT_1_BIT;
                    desc.loadOp = attachmentNode->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
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
                    desc.loadOp = attachmentNode->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    desc.finalLayout = attachmentNode->swapChain ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
                attachmentsDescriptions.push_back(desc);
            }

            // merge into global
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
    }

    std::vector<VkSubpassDependency> dependencies;
    // resolve dependencies
    auto topo = graph->Topo();

    for (auto [level, subPassNodes] : topo.levelsRenderPassOnly)
    {
        for (auto &subPassNode : subPassNodes)
        {
            auto subPassNodeSubPassIndex = std::find(graphicRenderPasses.begin(), graphicRenderPasses.end(), subPassNode) - graphicRenderPasses.begin();

            // find direct dependency only
            auto dependencees = subPassNode->TraceAllOutputs(GraphNode::GRAPHIC_PASS, 1);

            for (auto dependencee : dependencees)
            {
                auto dependenceeIndex = std::find(graphicRenderPasses.begin(), graphicRenderPasses.end(), dependencee) - graphicRenderPasses.begin();
                auto dependency = BuildDependency(subPassNodeSubPassIndex, dependenceeIndex, attachmentReferencesMap[subPassNode->name], attachmentReferencesMap[dependencee->name]);
                if (dependency.has_value())
                {
                    dependencies.push_back(dependency.value());
                }
            }

            VkSubpassDependency dependency = {};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency.srcAccessMask |= 0;
            dependency.dstStageMask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependency.dstAccessMask |= VK_ACCESS_SHADER_READ_BIT;
            dependencies.push_back(dependency);
        }
    }

    VkRenderPassCreateInfo renderPassInfoCI = {};
    renderPassInfoCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfoCI.attachmentCount = (uint32_t)(attachmentsDescriptions.size());
    renderPassInfoCI.pAttachments = attachmentsDescriptions.data();
    renderPassInfoCI.subpassCount = subPassDescriptions.size();
    renderPassInfoCI.pSubpasses = subPassDescriptions.data();
    renderPassInfoCI.dependencyCount = uint32_t(dependencies.size());
    renderPassInfoCI.pDependencies = dependencies.data();

    auto result = vkCreateRenderPass(context->GetVkDevice(), &renderPassInfoCI, nullptr, &this->renderPass);
}