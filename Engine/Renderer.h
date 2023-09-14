#pragma once

#include <vector>
#include <unordered_set>
#include <chrono>

#include <Core/IntrusivePtr.h>

#include <Engine/Window.h>
#include <Engine/Event.h>
#include <Engine/Camera.h>

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

    IntrusivePtr<Camera> GetCamera();

    void Build();

    void PostFrame();

    void Frame();

    void Update();

    bool Stopped();

    void RegisterUpdateCallback(UpdateCallback callback)
    {
        updateCallbacks.push_back(callback);
    }

private:
    PixelEngine* engine;
    IntrusivePtr<SwapChain> swapChain;
    IntrusivePtr<Window> window;
    IntrusivePtr<Camera> camera;
    std::vector<IntrusivePtr<ResourceBindingState>> drawStates;

    IntrusivePtr<RenderPassExecutor> renderPassExecutor;

    IOState ioState;

    uint64_t deltaTime = 0;

    std::vector<UpdateCallback> updateCallbacks;

    void InitWindow();
    void ReCreateSwapChain(uint32_t width, uint32_t height);
    void EventCallback(Event event);

    std::chrono::steady_clock::time_point frameStartTime;
};
