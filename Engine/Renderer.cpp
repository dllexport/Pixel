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

    Event event = {};
    event.type = Event::RESIZE;
    event.resizeEvent.width = width;
    event.resizeEvent.height = height;
    EventCallback(event);

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
}

void Renderer::Update()
{
    // TODO: opt resource update with frame overlap
    renderPassExecutor->WaitIdle();

    window->Update();

    // update call back may involved resource change
    Event event = {};
    event.type = Event::FRAME;
    EventCallback(event);

    renderPassExecutor->Update();
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
    this->ioState.Update(event);
    UpdateInput updateInput = {.event = event,
                               .deltaTime = deltaTime,
                               .ioState = ioState};
    for (auto &drawState : drawStates)
    {
        drawState->Update(updateInput);
    }

    // update renderer scope callbacks, camera
    for (auto cb : updateCallbacks) {
        cb(updateInput);
    }
}

IntrusivePtr<Camera> Renderer::GetCamera()
{
    if (!this->camera)
    {
        camera = new Camera(engine->GetRHIRuntime());
        camera->type = Camera::CameraType::firstperson;
        camera->setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
        camera->setRotation(glm::vec3(0.0f));
        camera->setPerspective(60.0f, (float)1024 / (float)768, 0.1f, 256.0f);
        camera->ubo.modelMatrix = glm::mat4(1.0f);
        camera->Sync();

        this->RegisterUpdateCallback(std::bind(&Camera::EventCallback, camera.get(), std::placeholders::_1));
    }

    return camera;
}