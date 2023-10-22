#pragma once

#include <RHI/RenderGroupExecutor.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Image.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/RenderGroup.h>

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

    virtual void AddBindingState(IntrusivePtr<ResourceBindingState> state) override;

private:
    std::unordered_set<IntrusivePtr<VulkanRenderPass>> renderPasses;

    IntrusivePtr<Context> context;

    std::unordered_map<std::string, IntrusivePtr<VulkanRenderGroup>> renderGroups;

    std::vector<VkFence> queueCompleteFences;

    void prepareFences();

    uint64_t currentFrame = 0;
    int32_t currentImage = 0;
};