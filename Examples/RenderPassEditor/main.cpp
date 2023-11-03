#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>

#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>
#include <Engine/Camera.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "EditorView.h"

int main()
{
    spdlog::set_level(spdlog::level::debug);

    IntrusivePtr<PixelEngine> engine = new PixelEngine;

    auto &rhiRuntime = engine->GetRHIRuntime();
    auto renderer = engine->CreateRenderer();

    IntrusivePtr<ImguiOverlay> ui = new EditorView(engine.get());
    ui->BuildPipeline();
    ui->BuildDrawable();

    renderer->AddDrawState(ui->drawState);

    engine->Frame();

    renderer.reset();
    engine.reset();

    return 0;
}