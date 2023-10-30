#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>

#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>
#include <Engine/ImguiOverlay.h>

#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);

    IntrusivePtr<PixelEngine> engine = new PixelEngine;

    auto &rhiRuntime = engine->GetRHIRuntime();
    auto renderer = engine->CreateRenderer();

    IntrusivePtr<ImguiOverlay> ui = new ImguiOverlay(engine.get());
    ui->BuildPipeline();
    ui->BuildDrawable();

    renderer->AddDrawState(ui->drawState);

    engine->Frame();

    renderer.reset();
    engine.reset();
    
    return 0;
}