#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>

#include <IO/KTXReader.h>

#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>
#include <Engine/Camera.h>
#include <Engine/ImguiOverlay.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

struct TextureUBO
{
    glm::mat4 model;
    float lodBias = 0.0f;
};

struct Vertex
{
    float pos[3];
    float uv[2];
    float normal[3];
};

std::vector<Vertex> vertexBuffer =
    {
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

std::vector<uint32_t> indexBuffer = {0, 1, 2, 2, 3, 0};
uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

void CreateTextureDrawable(PixelEngine *engine, IntrusivePtr<Renderer> renderer, IntrusivePtr<Pipeline> pipeline)
{
    auto rhiRuntime = engine->GetRHIRuntime();

    auto camera = renderer->GetCamera();

    auto vBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
    memcpy(vBuffer->Map(), vertexBuffer.data(), vertexBufferSize);
    auto iBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
    memcpy(iBuffer->Map(), indexBuffer.data(), indexBufferSize);

    auto vBufferGPU = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT | Buffer::BUFFER_USAGE_TRANSFER_DST_BIT, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBufferSize);
    auto iBufferGPU = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT | Buffer::BUFFER_USAGE_TRANSFER_DST_BIT, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBufferSize);

    engine->GetAuxiliaryExecutor()->TransferResource(vBufferGPU, vBuffer);
    engine->GetAuxiliaryExecutor()->TransferResource(iBufferGPU, iBuffer);
    vBuffer.reset();
    iBuffer.reset();

    auto texture = KTXReader::ReadFile(engine, "C:/Users/Mario/Documents/GitHub/Vulkan/data/textures/metalplate01_rgba.ktx");
    auto textureSampler = rhiRuntime->CreateSampler(texture);

    TextureUBO textureUBO = {
        .model = glm::mat4(1.5f),
        .lodBias = 1.0f,
    };
    auto uBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(TextureUBO));
    memcpy(uBuffer->Map(), &textureUBO, sizeof(textureUBO));

    auto rbs = rhiRuntime->CreateResourceBindingState(pipeline);
    rbs->Bind(0, 0, camera->GetUBOBuffer());
    rbs->Bind(0, 1, uBuffer);
    rbs->Bind(0, 2, textureSampler);
    rbs->BindVertexBuffer(vBufferGPU);
    rbs->BindIndexBuffer(iBufferGPU, ResourceBindingState::INDEX_TYPE_UINT32);
    rbs->BindDrawOp({ResourceBindingState::DrawOP{
        .indexCount = uint32_t(indexBuffer.size()),
        .instanceCount = 1,
        .firstIndex = 0,
        .vertexOffset = 0,
        .firstInstance = 0}});

    renderer->AddDrawState(rbs);
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    auto graph = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/Examples/Texture/texture.json");
    PipelineStates colorPipelineStates = {
        .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
        .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
        .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true}};

    IntrusivePtr<PixelEngine> engine = new PixelEngine;

    auto renderPass = engine->RegisterRenderPass(graph);
    auto colorPipeline = engine->RegisterPipeline("singlePass", "texture", colorPipelineStates);

    auto &rhiRuntime = engine->GetRHIRuntime();
    auto renderer = engine->CreateRenderer();

    IntrusivePtr<ImguiOverlay> ui = new ImguiOverlay(engine.get());
    ui->BuildPipeline();
    ui->BuildDrawable();

    renderer->AddDrawState(ui->drawState);

    CreateTextureDrawable(engine.get(), renderer, colorPipeline);

    engine->Frame();

    renderer.reset();
    engine.reset();

    return 0;
}