#pragma once

#include <unordered_set>
#include <Core/IntrusivePtr.h>
#include <RHI/Executor.h>
#include <RHI/RenderPass.h>
#include <RHI/ResourceBindingState.h>
#include <RHI/Texture.h>
#include <RHI/Pipeline.h>
#include <RHI/SwapChain.h>

class RenderPassExecutor : public Executor
{
public:
    RenderPassExecutor() = default;

    virtual ~RenderPassExecutor() = default;

    virtual uint32_t CurrentImage() = 0;

    // get swapchain image index
    virtual uint32_t Acquire() = 0;

    // allocate resource, attachments, framebuffer etc.
    virtual void Prepare() = 0;

    // update command buffer if any
    virtual void Update() = 0;

    // reset to init state when swapchain recreate
    virtual void Reset() = 0;

    void AddBindingState(IntrusivePtr<ResourceBindingState> state)
    {
        renderPasses.insert(state->GetPipeline()->GetRenderPass());
        resourceBindingStates[state->GetPipeline()].push_back(state);
    }

    void SetSwapChain(IntrusivePtr<SwapChain> swapChain)
    {
        this->swapChain = swapChain;
    }

protected:
    std::unordered_set<IntrusivePtr<RenderPass>> renderPasses;
    std::unordered_map<IntrusivePtr<Pipeline>, std::vector<IntrusivePtr<ResourceBindingState>>> resourceBindingStates;
    IntrusivePtr<SwapChain> swapChain;
};