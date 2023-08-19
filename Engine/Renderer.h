#pragma once

#include <vector>
#include <unordered_set>
#include <chrono>

#include <Core/IntrusivePtr.h>

#include <Engine/Window.h>
#include <Engine/Event.h>

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

    void PostFrame();

    void Frame();

    void Update();

    bool Stopped();

    template <class FN>
    void RegisterUpdateCallback(IntrusivePtr<ResourceBindingState> state, FN fn)
    {
        updateCallbacks[state] = fn;
    }

    struct UpdateInputs {
        Event event;
        uint64_t deltaTime;
        IntrusivePtr<ResourceBindingState> rbs;
    };

private:
    IntrusivePtr<PixelEngine> engine;
    IntrusivePtr<SwapChain> swapChain;
    IntrusivePtr<Window> window;
    std::vector<IntrusivePtr<ResourceBindingState>> drawStates;

    IntrusivePtr<RenderPassExecutor> renderPassExecutor;

    IOState ioState;

    std::unordered_map<IntrusivePtr<ResourceBindingState>, std::function<void(UpdateInputs)>> updateCallbacks;

    uint64_t deltaTime = 0;

    void InitWindow();
    void ReCreateSwapChain(uint32_t width, uint32_t height);
    void EventCallback(Event event);
};
