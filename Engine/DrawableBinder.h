#pragma once

#include <string>
#include <array>
#include <unordered_map>
#include <unordered_set>

#include <Core/IntrusivePtr.h>

#include <Engine/Renderable.h>
#include <RHI/ResourceHandle.h>
#include <RHI/RenderPass.h>
#include <RHI/Pipeline.h>
#include <RHI/ResourceBindingState.h>

class DrawableBinder : public IntrusiveCounter<DrawableBinder>
{
public:
    void BindResource(IntrusivePtr<Engine::Renderable> renderable, IntrusivePtr<ResourceBindingState> bindingState);

    void UnbindAll(IntrusivePtr<Engine::Renderable> renderable);

    // // a renderable may bind to different renderpass || pipelines
    // const auto &GetRenderableBindingState(IntrusivePtr<Engine::Renderable> renderable)
    // {
    //     return this->renderablesToBindingStates[renderable];
    // }

    // const auto& GetBoundRenderPassMap(IntrusivePtr<RenderPass> renderPass) {
    //     return this->renderPassToRenderables;
    // }

private:
    friend class Renderer;
    std::unordered_map<IntrusivePtr<Pipeline>, std::unordered_set<IntrusivePtr<Engine::Renderable>>> pipelineToRenderables;
    std::unordered_map<IntrusivePtr<Engine::Renderable>, std::unordered_set<IntrusivePtr<ResourceBindingState>>> renderablesToResourceBindingStates;
};
