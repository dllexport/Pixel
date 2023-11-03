#pragma once

#include <RHI/RenderGroupExecutor.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Image.h>
#include <RHI/VulkanRuntime/GraphicPass.h>
#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/RenderGroup.h>
#include <RHI/VulkanRuntime/AuxiliaryExecutor.h>
#include <vulkan/vulkan.h>

class VulkanGroupExecutor : public RenderGroupExecutor
{
public:
    VulkanGroupExecutor(IntrusivePtr<Context> context);
    virtual ~VulkanGroupExecutor() override;

    virtual uint32_t CurrentImage() override;
    virtual uint32_t Acquire() override;
    virtual void Prepare() override;
    virtual bool Execute() override;
    // rebuild command buffer
    virtual void Update() override;
    virtual void WaitIdle() override;
    virtual void Reset() override;

    virtual void AddRenderGroup(IntrusivePtr<RenderGroup> group) override;

    virtual void BindResource(std::string name, std::vector<IntrusivePtr<ResourceHandle>> resources) override;

private:
    IntrusivePtr<Context> context;
    // command buffers for global operations
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> renderCommandBuffers;

    std::unordered_set<IntrusivePtr<VulkanGraphicPass>> renderPasses;

    std::unordered_map<std::string, IntrusivePtr<VulkanRenderGroup>> renderGroups;

    // resource name (local) -> resource
    std::unordered_map<std::string, std::vector<IntrusivePtr<ResourceHandle>>> sharedResources;

    std::vector<VkFence> queueCompleteFences;

    void prepareFences();
    void releaseFences();
    // allocate all shared resources (per frame), store in sharedResources
    void prepareSharedResources();
    void releaseSharedResources();

    void prepareRenderCommandBuffers();
    void releaseRenderCommandBuffers();
    std::vector<VkCommandBuffer> createCommandBuffer(uint32_t size);

    // resource sync between render group execution
    void prepareRenderGroupSynchronization();

    // set swapchain texture layout before and after render
    // GENERAL -> PRESENT_SRC || PRESENT_SRC -> GENERAL
    void prepareGlobalSynchronization();

    struct SynCommands {
        // command per frame
        std::vector<VkCommandBuffer> beforeGroupExec;
        std::vector<VkCommandBuffer> afterGroupExec;
    };

    std::unordered_map<std::string, SynCommands> renderGroupSyncMap;
    SynCommands globalSynCommands;

    uint64_t currentFrame = 0;
    int32_t currentImage = 0;
};