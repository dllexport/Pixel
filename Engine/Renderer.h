#pragma once

#include <vector>
#include <unordered_set>

#include <Core/IntrusivePtr.h>
#include <Engine/Renderable.h>
#include <RHI/RenderPass.h>
#include <RHI/ResourceBindingState.h>

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

    void Update();

    void Frame();

private:
    PixelEngine *engine = nullptr;
    std::vector<IntrusivePtr<ResourceBindingState>> drawStates;
};
