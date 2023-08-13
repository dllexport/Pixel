#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>
#include <Engine/Renderer.h>
#include <spdlog/spdlog.h>
#include <Engine/Pixelengine.h>
#include <Engine/Renderable.h>

#include <Engine/Camera.h>
#include <glm/glm.hpp>

struct Vertex
{
    float position[3];
    float color[3];
};

struct
{
    glm::mat4 projectionMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
} uboVS;

std::vector<Vertex> vertexBuffer =
    {
        {{1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};
uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

std::vector<uint32_t> indexBuffer = {0, 1, 2};
uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

int main()
{
    spdlog::set_level(spdlog::level::debug);

    {
        auto graph = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/simple.json");
        PipelineStates pipelineStates = {
            .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
            .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
            .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true},
        };

        IntrusivePtr<PixelEngine> engine = new PixelEngine;
        auto drawableBinder = engine->GetDrawableBinder();

        auto renderPass = engine->RegisterRenderPass(graph);
        auto pipeline = engine->RegisterPipeline("singlePass", "single", pipelineStates);

        auto renderer = engine->CreateRenderer();

        auto vBuffer = engine->GetRHIRuntime()->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
        memcpy(vBuffer->Map(), vertexBuffer.data(), vertexBufferSize);
        auto iBuffer = engine->GetRHIRuntime()->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
        memcpy(iBuffer->Map(), indexBuffer.data(), indexBufferSize);
        auto uBuffer = engine->GetRHIRuntime()->CreateBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uboVS));

        Camera camera;
        camera.type = Camera::CameraType::lookat;
        camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
        camera.setRotation(glm::vec3(0.0f));
        camera.setPerspective(60.0f, (float)1024 / (float)768, 1.0f, 256.0f);

        uboVS.projectionMatrix = camera.matrices.perspective;
        uboVS.viewMatrix = camera.matrices.view;
        uboVS.modelMatrix = glm::mat4(1.0f);

        memcpy(uBuffer->Map(), &uboVS, sizeof(uboVS));

        auto renderable = new Engine::Renderable();

        auto rbs = engine->GetRHIRuntime()->CreateResourceBindingState(pipeline);
        rbs->Bind(0, 0, uBuffer);
        rbs->BindVertexBuffer(vBuffer);
        rbs->BindIndexBuffer(iBuffer);

        double i = 0;
        // engine->GetDrawableBinder()->BindResource(renderable, rbs);
        renderer->RegisterUpdateCallback([&]()
                                         {
                                         i += 0.0001;
                                         glm::mat4 trans(1.0f);
                                         trans = glm::scale(trans, {cos(i), cos(i), cos(i)});
                                         uboVS.modelMatrix = trans;
                                         memcpy(uBuffer->Map(), &uboVS, sizeof(uboVS));
                                         uBuffer->Dirty(); });
        renderer->AddDrawState(rbs);

        engine->Frame();

        int di = 0;
    }

    return 0;
}