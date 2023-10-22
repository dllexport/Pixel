#include <Engine/Renderer.h>
#include <Engine/PixelEngine.h>

#include <RHI/RenderGroupExecutor.h>

#include <spdlog/spdlog.h>

Renderer::Renderer(PixelEngine *engine) : engine(engine)
{
    InitWindow();
}

Renderer::~Renderer()
{
    renderGroupExecutor->WaitIdle();
    renderGroupExecutor->Reset();

    assert(renderGroupExecutor->use_count() == 1);
    renderGroupExecutor.reset();

    updateCallbacks.clear();

    for (auto &drawState : drawStates)
    {
        drawState->ClearUpdateCallbacks();
        drawState.reset();
    }

    drawStates.clear();

    this->window.reset();
    this->swapChain.reset();
    this->camera.reset();
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
    renderGroupExecutor->WaitIdle();

    Event event = {};
    event.type = Event::RESIZE;
    event.resizeEvent.width = width;
    event.resizeEvent.height = height;
    EventCallback(event);

    swapChain = engine->rhiRuntime->CreateSwapChain(this->window->hwnd, width, height);
    renderGroupExecutor->Reset();
    renderGroupExecutor->SetSwapChain(swapChain);
    renderGroupExecutor->Prepare();
    renderGroupExecutor->Acquire();
}

void Renderer::Build()
{
    renderGroupExecutor = engine->GetRHIRuntime()->CreateRenderGroupExecutor();
    for (auto &drawState : drawStates)
    {
        auto renderGroup = engine->renderGroupTemplates[drawState->GetPipeline()->groupName];
        renderGroup->AddBindingState(drawState);
        renderGroupExecutor->AddRenderGroup(renderGroup);
    }

    renderGroupExecutor->SetSwapChain(swapChain);
    renderGroupExecutor->Prepare();
}

void Renderer::AddDrawState(IntrusivePtr<ResourceBindingState> state)
{
    drawStates.push_back(state);
    
}

void Renderer::PostFrame()
{
}

void Renderer::Update()
{
    frameStartTime = std::chrono::high_resolution_clock::now();

    // acquire image so that it's save to update resource at imageIndex
    renderGroupExecutor->Acquire();

    window->Update();

    // update call back may involved resource change
    Event event = {};
    event.type = Event::FRAME;
    EventCallback(event);

    renderGroupExecutor->Update();
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
    auto executeResult = renderGroupExecutor->Execute();
    auto now = std::chrono::high_resolution_clock::now();
    this->deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameStartTime).count();
}

void Renderer::EventCallback(Event event)
{
    this->ioState.Update(event);
    UpdateInput updateInput = {.event = event,
                               .deltaTime = deltaTime,
                               .ioState = ioState,
                               .currentImageIndex = renderGroupExecutor->CurrentImage()};

    std::vector<UpdateCallback> groupCallbacks;
    for (auto &drawState : drawStates)
    {
        groupCallbacks.insert(groupCallbacks.end(), drawState->updateCallbacks.begin(), drawState->updateCallbacks.end());
    }

    for (auto cb : updateCallbacks)
    {
        groupCallbacks.insert(groupCallbacks.end(), updateCallbacks.begin(), updateCallbacks.end());
    }

    std::sort(groupCallbacks.begin(), groupCallbacks.end(), [](UpdateCallback &left, UpdateCallback &right)
              { return left.priority > right.priority; });

    // update renderer scope callbacks, camera
    for (auto cb : groupCallbacks)
    {
        if (cb.callback(updateInput))
        {
            break;
        }
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

        this->RegisterUpdateCallback({GENERAL, std::bind(&Camera::EventCallback, camera.get(), std::placeholders::_1)});
    }

    return camera;
}