#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>

#include <spdlog/spdlog.h>

#include <RHI/RenderPass.h>
#include <RHI/Pipeline.h>

PixelEngine::PixelEngine()
{
    rhiRuntime = RuntimeEntry(RuntimeEntry::Type::VULKAN).Create();
    drawableBinder = new DrawableBinder;
}

IntrusivePtr<RenderPass> PixelEngine::RegisterRenderPass(IntrusivePtr<Graph> graph)
{
    if (renderPassTemplates.count(graph->name))
    {
        spdlog::info("renderPass {} already exist", graph->name);
        return nullptr;
    }

    auto renderpass = rhiRuntime->CreateRenderPass(graph);
    renderpass->Build();
    this->renderPassTemplates[renderpass->Name()] = renderpass;

    return renderpass;
}

IntrusivePtr<Pipeline> PixelEngine::RegisterPipeline(std::string renderPassName, std::string subPassName, PipelineStates pipelineStates)
{
    if (!renderPassTemplates.count(renderPassName))
    {
        spdlog::info("renderPass {} does no exist", renderPassName);
        return nullptr;
    }
    auto renderPass = renderPassTemplates[renderPassName];
    auto &pipelineTemplate = this->pipelineTemplates[renderPass];

    auto pipeline = rhiRuntime->CreatePipeline(renderPass, subPassName, pipelineStates);
    pipeline->Build();
    pipelineTemplate[subPassName] = pipeline;

    return pipeline;
}

IntrusivePtr<RHIRuntime> PixelEngine::GetRHIRuntime()
{
    return rhiRuntime;
}

IntrusivePtr<DrawableBinder> PixelEngine::GetDrawableBinder()
{
    return drawableBinder;
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

    while (1)
    {
        for (auto &renderer : renderers)
        {
            renderer->Update();
            renderer->Frame();
        }
    }
}
