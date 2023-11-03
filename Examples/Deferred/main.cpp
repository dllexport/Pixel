#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>

#include <IO/KTXReader.h>
#include <IO/GLTFReader.h>

#include <Engine/PixelEngine.h>
#include <Engine/Renderer.h>
#include <Engine/Camera.h>

#include <Engine/ImguiOverlay.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

struct TextureUBO
{
    glm::mat4 model;
};

struct Light
{
    glm::vec4 position;
    glm::vec3 color;
    float radius;
};

struct LightsUBO
{
    Light lights[6];
    glm::vec4 viewPos;
    int debugDisplayTarget = 0;
};

void CreateTextureDrawable(PixelEngine *engine, IntrusivePtr<Renderer> renderer, IntrusivePtr<Pipeline> pipeline)
{
    auto rhiRuntime = engine->GetRHIRuntime();
    auto camera = renderer->GetCamera();

    auto gltfModels = GLTFReader::ReadFile(engine, "C:/Users/Mario/Desktop/Pixel/Examples/Deferred/assets/armor.gltf");

    for (auto model : gltfModels)
    {
        uint32_t vertexBufferSize = model.vertexBuffer.size() * sizeof(GLTFVertex);
        uint32_t indexBufferSize = model.indexBuffer.size() * sizeof(uint32_t);

        auto vBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
        memcpy(vBuffer->Map(), model.vertexBuffer.data(), vertexBufferSize);
        auto iBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
        memcpy(iBuffer->Map(), model.indexBuffer.data(), indexBufferSize);
        auto vBufferGPU = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT | Buffer::BUFFER_USAGE_TRANSFER_DST_BIT, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBufferSize);
        auto iBufferGPU = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT | Buffer::BUFFER_USAGE_TRANSFER_DST_BIT, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBufferSize);
        engine->GetAuxiliaryExecutor()->TransferResource(vBufferGPU, vBuffer);
        engine->GetAuxiliaryExecutor()->TransferResource(iBufferGPU, iBuffer);
        vBuffer.reset();
        iBuffer.reset();

        spdlog::info("colorTexture");
        auto colorTexture = KTXReader::ReadFile(engine, "C:/Users/Mario/Desktop/Pixel/Examples/Deferred/assets/colormap_rgba.ktx");
        spdlog::info("normalTexture");
        auto normalTexture = KTXReader::ReadFile(engine, "C:/Users/Mario/Desktop/Pixel/Examples/Deferred/assets/normalmap_rgba.ktx");
        auto colorTextureSampler = rhiRuntime->CreateSampler(colorTexture);
        auto normalTextureSampler = rhiRuntime->CreateSampler(normalTexture);

        TextureUBO textureUBO = {
            .model = glm::mat4(1.5f)};
        auto uBuffer = rhiRuntime->CreateMutableBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(TextureUBO));
        memcpy(uBuffer->GetBuffer(0)->Map(), &textureUBO, sizeof(textureUBO));

        auto rbs = rhiRuntime->CreateResourceBindingState(pipeline);
        rbs->Bind(0, 0, camera->GetUBOBuffer());
        rbs->Bind(0, 1, uBuffer);
        rbs->Bind(0, 2, colorTextureSampler);
        rbs->Bind(0, 3, normalTextureSampler);
        rbs->BindVertexBuffer(vBufferGPU);
        rbs->BindIndexBuffer(iBufferGPU, ResourceBindingState::INDEX_TYPE_UINT32);

        std::vector<ResourceBindingState::DrawOP> drawOps;
        for (auto primitive : model.primitives)
        {
            ResourceBindingState::DrawOP drawOP = {
                .indexCount = primitive.indexCount,
                .instanceCount = 1,
                .firstIndex = primitive.firstIndex,
                .vertexOffset = (int32_t)primitive.firstVertex,
                .firstInstance = 0};
            drawOps.push_back(drawOP);
        }
        rbs->BindDrawOp(drawOps);

        renderer->AddDrawState(rbs);
    }
}

void CreateComposeDrawable(PixelEngine *engine, IntrusivePtr<Renderer> renderer, IntrusivePtr<Pipeline> pipeline)
{
    auto rhiRuntime = engine->GetRHIRuntime();
    auto rbs = rhiRuntime->CreateResourceBindingState(pipeline);

    LightsUBO lightsUBO = {};
    // White
    lightsUBO.lights[0].position = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    lightsUBO.lights[0].color = glm::vec3(1.5f);
    lightsUBO.lights[0].radius = 15.0f * 0.25f;
    // Red
    lightsUBO.lights[1].position = glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f);
    lightsUBO.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
    lightsUBO.lights[1].radius = 15.0f;
    // Blue
    lightsUBO.lights[2].position = glm::vec4(2.0f, -1.0f, 0.0f, 0.0f);
    lightsUBO.lights[2].color = glm::vec3(0.0f, 0.0f, 2.5f);
    lightsUBO.lights[2].radius = 5.0f;
    // Yellow
    lightsUBO.lights[3].position = glm::vec4(0.0f, -0.9f, 0.5f, 0.0f);
    lightsUBO.lights[3].color = glm::vec3(1.0f, 1.0f, 0.0f);
    lightsUBO.lights[3].radius = 2.0f;
    // Green
    lightsUBO.lights[4].position = glm::vec4(0.0f, -0.5f, 0.0f, 0.0f);
    lightsUBO.lights[4].color = glm::vec3(0.0f, 1.0f, 0.2f);
    lightsUBO.lights[4].radius = 5.0f;
    // Yellow
    lightsUBO.lights[5].position = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    lightsUBO.lights[5].color = glm::vec3(1.0f, 0.7f, 0.3f);
    lightsUBO.lights[5].radius = 25.0f;

    double timer = 0;

    lightsUBO.lights[0].position.x = sin(glm::radians(360.0f * timer)) * 5.0f;
    lightsUBO.lights[0].position.z = cos(glm::radians(360.0f * timer)) * 5.0f;

    lightsUBO.lights[1].position.x = -4.0f + sin(glm::radians(360.0f * timer) + 45.0f) * 2.0f;
    lightsUBO.lights[1].position.z = 0.0f + cos(glm::radians(360.0f * timer) + 45.0f) * 2.0f;

    lightsUBO.lights[2].position.x = 4.0f + sin(glm::radians(360.0f * timer)) * 2.0f;
    lightsUBO.lights[2].position.z = 0.0f + cos(glm::radians(360.0f * timer)) * 2.0f;

    lightsUBO.lights[4].position.x = 0.0f + sin(glm::radians(360.0f * timer + 90.0f)) * 5.0f;
    lightsUBO.lights[4].position.z = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

    lightsUBO.lights[5].position.x = 0.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 10.0f;
    lightsUBO.lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * timer - 45.0f)) * 10.0f;

    lightsUBO.viewPos = glm::vec4(renderer->GetCamera()->position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

    auto siz = sizeof(LightsUBO);
    auto uBuffer = rhiRuntime->CreateMutableBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(LightsUBO));

    memcpy(uBuffer->GetBuffer(0)->Map(), &lightsUBO, sizeof(lightsUBO));

    rbs->Bind(0, 0, uBuffer);
    ResourceBindingState::DrawOP drawOP = {};
    drawOP.vertexCount = 3;
    drawOP.instanceCount = 1;
    rbs->BindDrawOp({drawOP});

    UpdateCallback updateCallback = {
        .priority = GENERAL,
        .callback = [uBuffer, camera = renderer->GetCamera()](UpdateInput input)
        {
            auto ubo = (LightsUBO *)uBuffer->GetBuffer(input.currentImageIndex)->Map();
            ubo->viewPos = camera->viewPos;
            return false;
        }};

    rbs->RegisterUpdateCallback(updateCallback);

    renderer->AddDrawState(rbs);
}

int main()
{
    spdlog::set_level(spdlog::level::debug);
    IntrusivePtr<PixelEngine> engine = new PixelEngine;

    auto graph = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/Examples/Deferred/deferred.json");

    PipelineStates colorPipelineStates = {
        .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
        .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
        .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true}};

    auto deferredRenderGroup = engine->RegisterRenderGroup(graph);
    auto colorPipeline = deferredRenderGroup->CreatePipeline("deferred", colorPipelineStates);
    auto composePipeline = deferredRenderGroup->CreatePipeline("compose", colorPipelineStates);

    auto& rhiRuntime = engine->GetRHIRuntime();
    auto renderer = engine->CreateRenderer();

    IntrusivePtr<ImguiOverlay> ui = new ImguiOverlay(engine.get());
    ui->BuildPipeline();
    ui->BuildDrawable();

    renderer->AddDrawState(ui->drawState);

    CreateTextureDrawable(engine.get(), renderer, colorPipeline);
    CreateComposeDrawable(engine.get(), renderer, composePipeline);

    engine->Frame();

    renderer.reset();
    engine.reset();

    return 0;
}