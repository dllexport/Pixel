#include <Engine/Renderer.h>

#include <Engine/PixelEngine.h>

#include <RHI/RenderPassExecutor.h>

Renderer::Renderer(PixelEngine *engine) : engine(engine)
{
    InitWindow();
}

Renderer::~Renderer()
{
}

void Renderer::InitWindow()
{
    this->window = new Window();
    this->window->Build(1024, 768);
    this->swapChain = engine->rhiRuntime->CreateSwapChain(this->window->GetHandle(), 1024, 768);
}

void Renderer::Build()
{
    renderPassExecutor = engine->GetRHIRuntime()->CreateRenderPassExecutor();
    for (auto &drawState : drawStates)
    {
        renderPassExecutor->AddBindingState(drawState);
    }

    renderPassExecutor->SetSwapChain(swapChain);
    renderPassExecutor->Prepare();
}

void Renderer::Update()
{
    window->Update();

    for (auto &cb : updateCallbacks)
    {
        cb();
    }
}

bool Renderer::Stopped()
{
    return window->Stopped();
}

void Renderer::Frame()
{
    if (window->Stopped())
    {
        return;
    }

    renderPassExecutor->Execute();
}