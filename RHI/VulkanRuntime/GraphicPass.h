#pragma once

#include <RHI/VulkanRuntime/Pass.h>
#include <vulkan/vulkan.h>

class VulkanGraphicPass : public VulkanPass
{
public:
    VulkanGraphicPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph);
    virtual ~VulkanGraphicPass() override;

    void Build(std::vector<std::string> subPasses);

    struct SubPassAttachmentReferences
    {
        std::vector<VkAttachmentReference> colorRefs;
        std::vector<VkAttachmentReference> depthRef;
        std::vector<VkAttachmentReference> inputRefs;

        uint64_t TotalSize()
        {
            return uint64_t(colorRefs.size() + depthRef.size() + inputRefs.size());
        }
    };

    VkRenderPass GetRenderPass();

    int32_t GetSubPassIndex(std::string subPassName);

    SubPassAttachmentReferences GetSubPassReference(std::string name);

    IntrusivePtr<GraphicRenderPassGraphNode> GetGraphicRenderPassGraphNode(uint32_t subPassIndex);
    IntrusivePtr<GraphicRenderPassGraphNode> GetGraphicRenderPassGraphNode(std::string subPassName);

private:
    friend class VulkanGraphicsPipeline;
    friend class VulkanRenderGroup;

    // for building command buffer in render group
    std::vector<IntrusivePtr<AttachmentGraphNode>> attachmentNodes;

    VkRenderPass renderPass;

    // subpass name -> references
    // storing input color VkAttachmentReference for each subpass
    std::unordered_map<std::string, SubPassAttachmentReferences> attachmentReferencesMap;
};