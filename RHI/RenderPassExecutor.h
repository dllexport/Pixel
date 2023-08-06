#pragma once

#include <unordered_set>
#include <Core/IntrusivePtr.h>
#include <RHI/RenderPass.h>
#include <RHI/ResourceBindingState.h>
#include <RHI/Texture.h>
#include <RHI/Pipeline.h>

class RenderPassExecutor : public IntrusiveCounter<RenderPassExecutor>
{
public:
    RenderPassExecutor(IntrusivePtr<RenderPass> renderPass)
    {
        this->renderPass = renderPass;
    }

    virtual ~RenderPassExecutor() = default;

    void AddBindingState(IntrusivePtr<ResourceBindingState> state)
    {
        resourceBindingStates[state->GetPipeline()].push_back(state);
    }

    void SetSwapChainExtent(Texture::Extent swapChainExtent)
    {
        this->swapChainExtent = swapChainExtent;
    }

    // import external attachment resource
    virtual void Import(IntrusivePtr<ResourceHandle> resource) = 0;

    // allocate resource, attachments, framebuffer etc.
    virtual void Prepare() = 0;

    // build command buffer
    virtual void Execute() = 0;

protected:
    IntrusivePtr<RenderPass> renderPass;
    std::unordered_map<IntrusivePtr<Pipeline>, std::vector<IntrusivePtr<ResourceBindingState>>> resourceBindingStates;
    Texture::Extent swapChainExtent;
};