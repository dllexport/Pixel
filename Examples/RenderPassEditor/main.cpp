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

    auto graph = Graph::ParseRenderPassJson("/Users/mario/Documents/GitHub/Pixel/Examples/RenderPassEditor/imgui.json");
    PipelineStates colorPipelineStates = {
        .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
        .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
        .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true}};

    IntrusivePtr<PixelEngine> engine = new PixelEngine;

    auto renderPass = engine->RegisterRenderPass(graph);

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