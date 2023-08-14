#pragma once

#include <unordered_set>
#include <Core/IntrusivePtr.h>
#include <RHI/RenderPass.h>
#include <RHI/ResourceBindingState.h>
#include <RHI/Texture.h>
#include <RHI/Pipeline.h>
#include <RHI/SwapChain.h>

class RenderPassExecutor : public IntrusiveCounter<RenderPassExecutor>
{
public:
    RenderPassExecutor()
    {
    }

    virtual ~RenderPassExecutor() = default;

    void AddBindingState(IntrusivePtr<ResourceBindingState> state)
    {
        renderPasses.insert(state->GetPipeline()->GetRenderPass());
        resourceBindingStates[state->GetPipeline()].push_back(state);
    }

    void SetSwapChain(IntrusivePtr<SwapChain> swapChain)
    {
        this->swapChain = swapChain;
    }

    // allocate resource, attachments, framebuffer etc.
    virtual void Prepare() = 0;

    // build command buffer
    virtual bool Execute() = 0;

    // update command buffer if any
    virtual void Update() = 0;

    virtual void WaitIdle() = 0;
    
    virtual void Reset() = 0;

protected:
    std::unordered_set<IntrusivePtr<RenderPass>> renderPasses;
    std::unordered_map<IntrusivePtr<Pipeline>, std::vector<IntrusivePtr<ResourceBindingState>>> resourceBindingStates;
    IntrusivePtr<SwapChain> swapChain;
};