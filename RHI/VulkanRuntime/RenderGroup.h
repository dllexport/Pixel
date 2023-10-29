#pragma once

#include <string>
#include <unordered_map>

#include <RHI/RenderGroup.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/ComputePass.h>
#include <RHI/VulkanRuntime/Image.h>
#include <RHI/VulkanRuntime/SwapChain.h>
#include <RHI/VulkanRuntime/ResourceBindingState.h>
#include <RHI/VulkanRuntime/AuxiliaryExecutor.h>

#include <vulkan/vulkan.h>

class VulkanRenderGroup : public RenderGroup
{
public:
    VulkanRenderGroup(IntrusivePtr<Context> context, IntrusivePtr<Graph> graph, IntrusivePtr<VulkanAuxiliaryExecutor> auxiliaryExecutor);
    virtual ~VulkanRenderGroup();

    // build renderpasses and compute pipeline
    virtual void Build() override;

    // prepare resource for the first time
    void Prepare(VulkanSwapChain *swapchain);

    void RegisterPipeline(std::string name, IntrusivePtr<VulkanGraphicsPipeline> pipeline);

    void ImportResource(std::string name, std::vector<IntrusivePtr<ResourceHandle>> resources);

    virtual void AddBindingState(IntrusivePtr<ResourceBindingState> state) override;

    // rebuild command buffers
    void Update(uint32_t currentImageIndex, VulkanSwapChain *swapChain);

    void Reset();

    std::vector<VkCommandBuffer> GetCommandBuffer(uint32_t currentImageIndex);

    IntrusivePtr<VulkanRenderPass> GetRenderPass(std::string name);

    IntrusivePtr<VulkanTexture> CreateAttachmentResource(VulkanSwapChain *swapChain, IntrusivePtr<AttachmentGraphNode> attachmentNode);

    static uint32_t DeferAttachmentUsage(IntrusivePtr<AttachmentGraphNode> attachmentNode);
    static VkImageAspectFlags DeferAttachmentAspect(IntrusivePtr<AttachmentGraphNode> attachmentNode);

private:
    IntrusivePtr<Context> context;
    IntrusivePtr<VulkanAuxiliaryExecutor> auxiliaryExecutor;

    std::unordered_map<std::string, IntrusivePtr<VulkanRenderPass>> renderPasses;
    std::unordered_map<std::string, IntrusivePtr<VulkanComputePass>> computePasses;

    struct RenderPassFrameResource
    {
        // attachment name -> vulkan image(1 per inflight)
        std::unordered_map<std::string, std::vector<VulkanImage>> attachmentImages;
        std::vector<VkFramebuffer> frameBuffers;
        std::vector<VkCommandBuffer> commandBuffers;
    };
    std::unordered_map<IntrusivePtr<VulkanRenderPass>, RenderPassFrameResource> renderPassResourceMap;

    struct ComputePassFrameResource
    {
        // resource name -> buffer
        std::unordered_map<std::string, std::vector<ResourceHandle>> buffers;
        std::vector<VkCommandBuffer> commandBuffers;
    };
    std::unordered_map<IntrusivePtr<VulkanComputePass>, ComputePassFrameResource> computePassResourceMap;

    // resources external import
    // global name -> res(per inflight)
    std::unordered_map<std::string, std::vector<IntrusivePtr<ResourceHandle>>> sharedResources;

    // resources in group scope
    // (group name::resource name) -> res(per inflight)
    std::unordered_map<std::string, std::vector<IntrusivePtr<ResourceHandle>>> groupScopeResources;

    // pipeline -> drawStates
    std::unordered_map<IntrusivePtr<Pipeline>, std::vector<IntrusivePtr<VulkanResourceBindingState>>> resourceBindingStates;

    // subpass name -> graphic, compute pipeline
    std::unordered_map<std::string, IntrusivePtr<Pipeline>> pipelineMap;

    VkCommandPool graphicCommandPool;
    VkCommandPool computeCommandPool;
    void prepareCommandPool();

    // create VkCommandBuffer and VkFramebuffer
    // 1 per frame
    void prepareCommandBuffer(IntrusivePtr<VulkanRenderPass> &renderPass, VulkanSwapChain *swapChain);
    
    // 1. layout transition from UNDEFINED to GENERAL for each attachment
    //    setting layout for shared attachment if restrained by inTransition state
    // 2. create texture and framebuffer (per frame)
    void prepareFrameBuffer(IntrusivePtr<VulkanRenderPass> &renderPass, VulkanSwapChain *swapChain);

    // record command buffer at imageIndex
    void buildCommandBuffer(uint32_t imageIndex, VulkanSwapChain *swapChain);

    // allocate all texture resource
    void resolveDrawStatesDescriptors(VulkanSwapChain *swapChain);
};