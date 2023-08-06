#pragma once

#include <RHI/RenderPass.h>
#include <RHI/VulkanRuntime/Context.h>

#include <vulkan/vulkan.h>

class VulkanPipeline;
class VulkanRenderPass : public RenderPass
{
public:
    VulkanRenderPass(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph);
    virtual ~VulkanRenderPass() override;

    virtual void Build() override;

    struct SubPassAttachmentReferences
    {
        std::vector<VkAttachmentReference> colorRefs;
        std::vector<VkAttachmentReference> depthRef;
        std::vector<VkAttachmentReference> inputRefs;
    };

    VkRenderPass GetRenderPass()
    {
        return renderPass;
    }

    int32_t GetSubPassIndex(std::string subPassName)
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

    void RegisterPipeline(std::string name, IntrusivePtr<VulkanPipeline> pipeline);

private:
    friend class VulkanPipeline;
    friend class VulkanRenderPassExecutor;
    IntrusivePtr<Context> context;
    VkRenderPass renderPass;
    std::vector<IntrusivePtr<GraphicRenderPassGraphNode>> graphicRenderPasses;

    struct AttachmentMiniDescription
    {
        VkFormat format;
        // VkImageUsageFlagBits
        VkFlags usage;
    };
    std::vector<AttachmentMiniDescription> attachmentMiniDescriptions;
    // subpass name -> references
    std::unordered_map<std::string, SubPassAttachmentReferences> referencesMap;

    std::unordered_map<std::string, IntrusivePtr<VulkanPipeline>> pipelineMap;
};