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

IntrusivePtr<Pipeline> PixelEngine::RegisterPipeline(std::string renderGroupName, std::string subPassName, PipelineStates pipelineStates)
{
    if (!renderGroupTemplates.count(renderGroupName))
    {
        spdlog::info("renderPass {} does no exist", renderGroupName);
        return nullptr;
    }
    auto renderGroup = renderGroupTemplates[renderGroupName];
    auto &pipelineTemplate = this->pipelineTemplates[renderGroup];

    auto pipeline = rhiRuntime->CreatePipeline(renderGroup, subPassName, pipelineStates);
    pipeline->Build();
    pipelineTemplate[subPassName] = pipeline;

    return pipeline;
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
