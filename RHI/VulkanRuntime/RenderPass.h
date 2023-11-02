#pragma once

#include <RHI/VulkanRuntime/Context.h>
#include <FrameGraph/Graph.h>
#include <vulkan/vulkan.h>

class VulkanRenderPass : public IntrusiveCounter<VulkanRenderPass>
{
public:
    VulkanRenderPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph);
    ~VulkanRenderPass();

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
    IntrusivePtr<Context> context;
    IntrusivePtr<Graph> graph;

    // ordered subpass node from 0 to n
    std::vector<IntrusivePtr<GraphicRenderPassGraphNode>> graphicRenderPasses;
    // for building command buffer in render group
    std::vector<IntrusivePtr<AttachmentGraphNode>> attachmentNodes;

    VkRenderPass renderPass;

    // subpass name -> references
    // storing input color VkAttachmentReference for each subpass
    std::unordered_map<std::string, SubPassAttachmentReferences> attachmentReferencesMap;
};