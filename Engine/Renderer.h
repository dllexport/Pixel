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

private:
    PixelEngine *engine = nullptr;
    IntrusivePtr<SwapChain> swapChain;
    IntrusivePtr<Window> window;
    std::vector<IntrusivePtr<ResourceBindingState>> drawStates;

    IntrusivePtr<RenderPassExecutor> renderPassExecutor;

    void InitWindow();
};
