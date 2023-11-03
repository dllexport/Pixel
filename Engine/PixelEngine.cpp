#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>

#include <spdlog/spdlog.h>

#include <RHI/RenderGroup.h>
#include <RHI/Pipeline.h>

PixelEngine::PixelEngine()
{
    rhiRuntime = RuntimeEntry(RuntimeEntry::Type::VULKAN).Create();
    this->auxExecutor = rhiRuntime->CreateAuxiliaryExecutor();
}

PixelEngine::~PixelEngine()
{
    assert(auxExecutor->use_count() == 1);
    assert(rhiRuntime->use_count() == 1);
}

IntrusivePtr<RenderGroup> PixelEngine::RegisterRenderGroup(IntrusivePtr<Graph> graph)
{
    if (renderGroupTemplates.count(graph->GetName()))
    {
        spdlog::info("renderPass {} already exist", graph->GetName());
        return nullptr;
    }

    auto renderGroup = rhiRuntime->CreateRenderGroup(graph);
    renderGroup->Build();
    this->renderGroupTemplates[renderGroup->Name()] = renderGroup;

    return renderGroup;
}

IntrusivePtr<RHIRuntime> &PixelEngine::GetRHIRuntime()
{
    return rhiRuntime;
}

IntrusivePtr<Renderer> PixelEngine::CreateRenderer()
{
    auto renderer = new Renderer(this);
    renderers.push_back(renderer);
    return renderer;
}

void PixelEngine::Frame()
{
    for (auto &renderer : renderers)
    {
        renderer->Build();
    }

    while (!renderers.empty())
    {
        for (auto it = renderers.begin(); it != renderers.end();)
        {
            auto renderer = *it;
            if (renderer->Stopped())
            {
                it = renderers.erase(it);
                continue;
            }

            // perform global update before renderer frame
            this->auxExecutor->Execute();

            renderer->Update();
            renderer->Frame();
            renderer->PostFrame();
            it++;
        }
    }
}
