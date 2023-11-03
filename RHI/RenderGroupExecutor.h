#pragma once

#include <unordered_set>

#include <Core/IntrusivePtr.h>

#include <RHI/Executor.h>
#include <RHI/RenderGroup.h>
#include <RHI/ResourceHandle.h>
#include <RHI/Texture.h>
#include <RHI/SwapChain.h>
#include <RHI/ResourceBindingState.h>

class RenderGroupExecutor : public Executor
{
public:
    RenderGroupExecutor() = default;

    virtual ~RenderGroupExecutor() = default;

    virtual uint32_t CurrentImage() = 0;

    // get swapchain image index
    virtual uint32_t Acquire() = 0;

    // allocate resource, attachments, framebuffer etc.
    // resolve synchronization commands between rendergroups
    virtual void Prepare() = 0;

    // update command buffer if any
    virtual void Update() = 0;

    // reset to init state when swapchain recreate
    virtual void Reset() = 0;

    virtual void AddRenderGroup(IntrusivePtr<RenderGroup> group) = 0;

    // bind global resources(per frame), shared among all rendergroups
    virtual void BindResource(std::string name, std::vector<IntrusivePtr<ResourceHandle>> resources) = 0;

    void SetSwapChain(IntrusivePtr<SwapChain> swapChain);
    IntrusivePtr<SwapChain> GetSwapChain();

protected:
    IntrusivePtr<SwapChain> swapChain;
};