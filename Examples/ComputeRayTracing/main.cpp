#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>

#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>
#include <Engine/Camera.h>

#include <Engine/ImguiOverlay.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

void CreatePresentDrawable(PixelEngine *engine, IntrusivePtr<Renderer> renderer, IntrusivePtr<Pipeline> pipeline)
{
    auto rhiRuntime = engine->GetRHIRuntime();
    auto rbs = rhiRuntime->CreateResourceBindingState(pipeline);

    ResourceBindingState::DrawOP drawOP = {};
    drawOP.vertexCount = 3;
    drawOP.instanceCount = 1;
    rbs->BindDrawOp({drawOP});

    renderer->AddDrawState(rbs);
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    auto graph = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/Examples/ComputeRayTracing/ComputeRayTracing.json");

    IntrusivePtr<PixelEngine> engine = new PixelEngine;
    auto renderGroup = engine->RegisterRenderGroup(graph);
    auto computePipeline = renderGroup->CreatePipeline("compute 0", ComputePipelineStates{});

    auto &rhiRuntime = engine->GetRHIRuntime();
    auto renderer = engine->CreateRenderer();

    IntrusivePtr<ImguiOverlay> ui = new ImguiOverlay(engine.get());
    ui->BuildPipeline();
    ui->BuildDrawable();

    renderer->AddDrawState(ui->drawState);

    PipelineStates presentPipelineStates = {
        .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
        .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
        .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true}};

    auto presentPipeline = renderGroup->CreatePipeline("present", presentPipelineStates);

    CreatePresentDrawable(engine.get(), renderer, presentPipeline);

    engine->Frame();

    renderer.reset();
    engine.reset();

    return 0;
}