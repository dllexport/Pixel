#include <Engine/Renderer.h>

#include <Engine/PixelEngine.h>

#include <RHI/RenderPassExecutor.h>

Renderer::Renderer(PixelEngine *engine) : engine(engine)
{
}

Renderer::~Renderer()
{
}

void Renderer::Frame()
{
    auto drawableBinder = engine->GetDrawableBinder();

    std::unordered_map<IntrusivePtr<RenderPass>, IntrusivePtr<RenderPassExecutor>> renderPassExecutors;

    for (auto &drawState : drawStates)
    {
        auto renderPass = drawState->GetPipeline()->GetRenderPass();
        if (!renderPassExecutors.count(renderPass))
        {
            renderPassExecutors[renderPass] = engine->GetRHIRuntime()->CreateRenderPassExecutor(renderPass);
        }

        renderPassExecutors[renderPass]->AddBindingState(drawState);
    }

    for (auto &[renderPass, executor] : renderPassExecutors)
    {
        executor->SetSwapChainExtent({1024, 768, 1});
        executor->Prepare();
        executor->Execute();
    }
}