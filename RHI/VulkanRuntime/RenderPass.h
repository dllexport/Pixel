#pragma once

#include <RHI/RenderPass.h>
#include <RHI/VulkanRuntime/Context.h>

#include <vulkan/vulkan.h>

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

private:
    friend class VulkanPipeline;
    IntrusivePtr<Context> context;
    VkRenderPass renderPass;
    std::vector<IntrusivePtr<GraphicRenderPassGraphNode>> graphicRenderPasses;
    std::vector<VkAttachmentDescription> attachments;
    // subpass name -> references
    std::unordered_map<std::string, SubPassAttachmentReferences> referencesMap;
};