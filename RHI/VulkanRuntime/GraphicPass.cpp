#include <RHI/VulkanRuntime/GraphicPass.h>
#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/Texture.h>

#include <spdlog/spdlog.h>

#include <optional>
#include <iterator>

VulkanGraphicPass::VulkanGraphicPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph) : VulkanPass(context, graph)
{
}

VulkanGraphicPass::~VulkanGraphicPass()
{
    vkDestroyRenderPass(context->GetVkDevice(), renderPass, nullptr);
}

VkRenderPass VulkanGraphicPass::GetRenderPass()
{
    return renderPass;
}

int32_t VulkanGraphicPass::GetSubPassIndex(std::string subPassName)
{
    for (int32_t i = 0; i < renderPassGraphNodes.size(); i++)
    {
        if (renderPassGraphNodes[i]->LocalName() == subPassName)
        {
            return i;
        }
    }
    return -1;
}

VulkanGraphicPass::SubPassAttachmentReferences VulkanGraphicPass::GetSubPassReference(std::string name)
{
    if (attachmentReferencesMap.count(name))
    {
        return attachmentReferencesMap[name];
    }
    return {};
}

IntrusivePtr<GraphicRenderPassGraphNode> VulkanGraphicPass::GetGraphicRenderPassGraphNode(uint32_t subPassIndex)
{
    return renderPassGraphNodes[subPassIndex]->As<GraphicRenderPassGraphNode *>();
}

IntrusivePtr<GraphicRenderPassGraphNode> VulkanGraphicPass::GetGraphicRenderPassGraphNode(std::string subPassName)
{
    return renderPassGraphNodes[GetSubPassIndex(subPassName)]->As<GraphicRenderPassGraphNode *>();
}

std::optional<VkSubpassDependency> BuildDependency(uint32_t fromIndex, uint32_t toIndex, VulkanGraphicPass::SubPassAttachmentReferences from, VulkanGraphicPass::SubPassAttachmentReferences to)
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

void VulkanGraphicPass::Build(std::vector<std::string> subPasses)
{
    // todo: handle multisubpassed
    std::vector<VkAttachmentDescription> attachmentsDescriptions;
    std::vector<VkSubpassDescription> subPassDescriptions;
    uint32_t attachmentCounter = 0;

    // may have multiple nodes in same level
    for (auto &subPass : subPasses)
    {
        auto &subPassNode = graph->GetNodeMap().at(subPass);
        this->renderPassGraphNodes.push_back(subPassNode->As<RenderPassGraphNode *>());

        // store the newly created attachment node in this pass
        std::vector<IntrusivePtr<AttachmentGraphNode>> subPassAttachmentNodes;

        auto &referencesGroup = this->attachmentReferencesMap[subPassNode->LocalName()];

        for (auto n : subPassNode->inputs)
        {
            if (n->type == GraphNode::ATTACHMENT)
            {
                auto agn = static_cast<AttachmentGraphNode *>(n.get());
                int attachmentRefIndex = std::find_if(attachmentNodes.begin(), attachmentNodes.end(), [&](IntrusivePtr<AttachmentGraphNode> &node)
                                                      { return node->GlobalName() == agn->GlobalName(); }) -
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
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
            }
            else
            {
                desc.format = GeneralFormatToVkFormat(attachmentNode->format);
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = attachmentNode->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
                desc.storeOp = attachmentNode->input ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
                desc.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
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

    // handle synchronization within pass
    std::vector<VkSubpassDependency> dependencies;
    // resolve dependencies
    auto topo = graph->Topo();

    bool externalDependencySet = false;
    for (auto &subPass : subPasses)
    {
        auto &subPassNode = graph->GetNodeMap().at(subPass);

        auto subPassNodeSubPassIndex = std::find(renderPassGraphNodes.begin(), renderPassGraphNodes.end(), subPassNode) - renderPassGraphNodes.begin();

        // find direct dependency only
        auto dependencees = subPassNode->TraceAllOutputs(GraphNode::GRAPHIC_PASS, 1);

        for (auto dependencee : dependencees)
        {
            auto dependenceeIndex = std::find(renderPassGraphNodes.begin(), renderPassGraphNodes.end(), dependencee) - renderPassGraphNodes.begin();
            auto dependency = BuildDependency(subPassNodeSubPassIndex, dependenceeIndex, attachmentReferencesMap[subPassNode->GlobalName()], attachmentReferencesMap[dependencee->GlobalName()]);
            if (dependency.has_value())
            {
                dependencies.push_back(dependency.value());
            }
        }

        for (auto output : subPassNode->outputs)
        {
            if (output->type != GraphNode::ATTACHMENT)
                continue;

            auto attachmentNode = output->As<AttachmentGraphNode *>();
            // Hazard READ_AFTER_WRITE
            // layout transition from GENERAL to DET (write)
            // loadOp VK_ATTACHMENT_LOAD_OP_LOAD (TODO: some color attachments maybe write only)
            if (attachmentNode->swapChain || attachmentNode->color)
            {
                VkSubpassDependency dependency{};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask = VK_ACCESS_NONE;
                dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                dependencies.push_back(dependency);
            }

            if (attachmentNode->depthStencil)
            {
                VkSubpassDependency dependency{};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                dependency.srcAccessMask = VK_ACCESS_NONE;
                dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                dependencies.push_back(dependency);
            }
        }

        for (auto input : subPassNode->inputs)
        {
            if (input->type != GraphNode::ATTACHMENT)
                continue;

            auto attachmentNode = input->As<AttachmentGraphNode *>();
            // Hazard READ_AFTER_WRITE
            // layout transition from GENERAL to DET (write)
            // loadOp VK_ATTACHMENT_LOAD_OP_LOAD (TODO: some color attachments maybe write only)
            if (attachmentNode->input)
            {
                VkSubpassDependency dependency{};
                dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass = 0;
                dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependency.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
                dependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
                dependencies.push_back(dependency);
                break;
            }
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
