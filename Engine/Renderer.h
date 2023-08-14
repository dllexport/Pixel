#pragma once

#include <vector>
#include <unordered_set>

#include <Core/IntrusivePtr.h>

#include <Engine/Window.h>

#include <RHI/RenderPass.h>
#include <RHI/ResourceBindingState.h>
#include <RHI/RenderPassExecutor.h>
#include <RHI/SwapChain.h>

class PixelEngine;
class Renderer : public IntrusiveCounter<Renderer>
{
public:
    Renderer(PixelEngine *engine);
    ~Renderer();

    void AddDrawState(IntrusivePtr<ResourceBindingState> state)
    {
        drawStates.push_back(state);
    }

    void Build();

    void Frame();

    void Update();

    bool Stopped();

    template <class FN>
    void RegisterUpdateCallback(FN fn)
    {
        updateCallbacks.push_back(fn);
    }

    void ReCreateSwapChain(uint32_t width, uint32_t height);

private:
    IntrusivePtr<PixelEngine> engine;
    IntrusivePtr<SwapChain> swapChain;
    IntrusivePtr<Window> window;
    std::vector<IntrusivePtr<ResourceBindingState>> drawStates;

    IntrusivePtr<RenderPassExecutor> renderPassExecutor;

    std::vector<std::function<void()>> updateCallbacks;

    void InitWindow();
};
