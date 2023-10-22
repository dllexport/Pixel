#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>

#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>
#include <Engine/Camera.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <Engine/ImguiOverlay.h>

struct Vertex
{
    float position[3];
    float color[3];
};

std::vector<Vertex> vertexBuffer =
    {
        {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};
uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

std::vector<uint32_t> indexBuffer = {0, 1, 2};
uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

void CreateTriangleDrawable(IntrusivePtr<RHIRuntime> rhiRuntime, IntrusivePtr<Renderer> renderer, IntrusivePtr<Pipeline> pipeline)
{
    auto camera = renderer->GetCamera();

    auto vBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
    memcpy(vBuffer->Map(), vertexBuffer.data(), vertexBufferSize);
    auto iBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
    memcpy(iBuffer->Map(), indexBuffer.data(), indexBufferSize);

    glm::mat4 model = glm::mat4(1.0f);

    auto uBuffer = rhiRuntime->CreateMutableBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(model));
    memcpy(uBuffer->GetBuffer(0)->Map(), &model, sizeof(model));

    auto rbs = rhiRuntime->CreateResourceBindingState(pipeline);
    rbs->Bind(0, 0, camera->GetUBOBuffer());
    rbs->Bind(0, 1, uBuffer);
    rbs->BindVertexBuffer(vBuffer);
    rbs->BindIndexBuffer(iBuffer, ResourceBindingState::INDEX_TYPE_UINT32);
    rbs->BindDrawOp({ResourceBindingState::DrawOP{
        .indexCount = 3,
        .instanceCount = 1,
        .firstIndex = 0,
        .vertexOffset = 0,
        .firstInstance = 1}});
    rbs->name = "triangle";
    renderer->AddDrawState(rbs);
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    auto graph = Graph::ParseRenderPassJson("triangle.json");
    PipelineStates colorPipelineStates = {
        .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
        .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
        .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true}};

    IntrusivePtr<PixelEngine> engine = new PixelEngine;
    // auto imguiRenderPass = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/Examples/RenderPassEditor/imgui.json");
    // auto renderPass2 = engine->RegisterRenderGroup(imguiRenderPass);
    auto renderPass = engine->RegisterRenderGroup(graph);
    auto colorPipeline = engine->RegisterPipeline("TrianglePass", "single", colorPipelineStates);

    auto &rhiRuntime = engine->GetRHIRuntime();
    auto renderer = engine->CreateRenderer();

    // IntrusivePtr<ImguiOverlay> ui = new ImguiOverlay(engine.get());
    // ui->BuildPipeline();
    // ui->BuildDrawable();

    // renderer->AddDrawState(ui->drawState);

    CreateTriangleDrawable(rhiRuntime, renderer, colorPipeline);

    engine->Frame();

    renderer.reset();
    engine.reset();

    return 0;
}