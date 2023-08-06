#include <Engine/DrawableBinder.h>

void DrawableBinder::BindResource(IntrusivePtr<Engine::Renderable> renderable, IntrusivePtr<ResourceBindingState> bindingState)
{
    auto pipeline = bindingState->GetPipeline();
    if (pipelineToRenderables.count(pipeline))
    {
        if (pipelineToRenderables[pipeline].count(renderable))
        {
            return;
        }
    }

    auto &resourceBindingStates = renderablesToResourceBindingStates[renderable];
    if (resourceBindingStates.count(bindingState))
    {
        return;
    }

    resourceBindingStates.insert(bindingState);
    pipelineToRenderables[pipeline].insert(renderable);
}

void DrawableBinder::UnbindAll(IntrusivePtr<Engine::Renderable> renderable)
{
}
