#include <Engine/Renderer.h>

#include <Engine/PixelEngine.h>

#include <RHI/RenderPassExecutor.h>

#include <spdlog/spdlog.h>

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
    this->window->RegisterResizeCallback([this](uint32_t width, uint32_t height)
                                         { this->ReCreateSwapChain(width, height); });

    this->swapChain = engine->rhiRuntime->CreateSwapChain(this->window->GetHandle(), 1024, 768);
}

void Renderer::ReCreateSwapChain(uint32_t width, uint32_t height)
{
    spdlog::info("recreate");
    this->renderPassExecutor->WaitIdle();
    this->swapChain = engine->rhiRuntime->CreateSwapChain(this->window->GetHandle(), width, height);
    renderPassExecutor->Reset();
    renderPassExecutor->SetSwapChain(swapChain);
    renderPassExecutor->Prepare();
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

    auto executeResult = renderPassExecutor->Execute();
    if (!executeResult)
    {
    }
}
