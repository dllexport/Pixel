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
    this->window->RegisterResizeCallback(std::bind(&Renderer::ReCreateSwapChain, this, std::placeholders::_1, std::placeholders::_2));
    this->window->RegisterEventCallback(std::bind(&Renderer::EventCallback, this, std::placeholders::_1));
    this->swapChain = engine->rhiRuntime->CreateSwapChain(this->window->hwnd, 1024, 768);
}

void Renderer::ReCreateSwapChain(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0)
    {
        return;
    }
    renderPassExecutor->WaitIdle();
    swapChain = engine->rhiRuntime->CreateSwapChain(this->window->hwnd, width, height);
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

void Renderer::PostFrame()
{
    // TODO: opt resource update with frame overlap
    renderPassExecutor->WaitIdle();

    window->Update();

    // update call back may involved resource change
    Event event = {};
    event.type = Event::FRAME;
    for (auto &[drawState, cb] : updateCallbacks)
    {
        UpdateInputs updateInputs = {
            .event = event,
            .deltaTime = deltaTime,
            .rbs = drawState};
        cb(updateInputs);
    }

    renderPassExecutor->Update();
}

void Renderer::Update()
{
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
    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    auto executeResult = renderPassExecutor->Execute();
    auto now = std::chrono::high_resolution_clock::now();
    this->deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFrameTime).count();
}

void Renderer::EventCallback(Event event)
{
    spdlog::info("{}", event.type);
    for (auto &[drawState, cb] : updateCallbacks)
    {
        UpdateInputs updateInputs = {
            .event = event,
            .deltaTime = deltaTime,
            .rbs = drawState};
        cb(updateInputs);
    }

    // switch (event.type)
    // {
    // case Event::KEY_DOWN:
    // {
    //     break;
    // }
    // case Event::KEY_UP:
    // {
    //     break;
    // }
    // case Event::KEY_REPEAT:
    // {
    //     break;
    // }
    // case Event::MOUSE_DOWN:
    // {
    //     break;
    // }
    // case Event::MOUSE_UP:
    // {
    //     break;
    // }
    // case Event::WINDOW_FOCUS_IN:
    // {
    //     break;
    // }
    // case Event::WINDOW_FOCUS_OUT:
    // {
    //     break;
    // }

    // default:
    //     break;
    // }
}
