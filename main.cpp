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
        PipelineStates colorPipelineStates = {
            .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
            .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
            .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true}};

        PipelineStates imguiPipelineStates = {
            .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
            .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
            .depthStencilState = {.depthTestEnable = false, .depthWriteEnable = false}};

        IntrusivePtr<PixelEngine> engine = new PixelEngine;

        auto renderPass = engine->RegisterRenderPass(graph);
        auto colorPipeline = engine->RegisterPipeline("singlePass", "single", colorPipelineStates);
        auto imguiPipeline = engine->RegisterPipeline("singlePass", "imgui", imguiPipelineStates);

        auto renderer = engine->CreateRenderer();

        auto vBuffer = engine->GetRHIRuntime()->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
        memcpy(vBuffer->Map(), vertexBuffer.data(), vertexBufferSize);
        auto iBuffer = engine->GetRHIRuntime()->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
        memcpy(iBuffer->Map(), indexBuffer.data(), indexBufferSize);
        auto uBuffer = engine->GetRHIRuntime()->CreateBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uboVS));

        Camera camera;
        camera.type = Camera::CameraType::firstperson;
        camera.setPosition(glm::vec3(0.0f, 0.0f, -1.5f));
        camera.setRotation(glm::vec3(0.0f));
        camera.setPerspective(60.0f, (float)1024 / (float)768, 1.0f, 256.0f);
        glm::vec2 mousePos = {};

        uboVS.projectionMatrix = camera.matrices.perspective;
        uboVS.viewMatrix = camera.matrices.view;
        uboVS.modelMatrix = glm::mat4(1.0f);

        memcpy(uBuffer->Map(), &uboVS, sizeof(uboVS));

        auto renderable = new Engine::Renderable();

        auto rbs = engine->GetRHIRuntime()->CreateResourceBindingState(colorPipeline);
        rbs->Bind(0, 0, uBuffer);
        rbs->BindVertexBuffer(vBuffer);
        rbs->BindIndexBuffer(iBuffer);

        renderer->RegisterUpdateCallback([&](Event event, uint64_t deltaTime)
                                         {
            if (event.type == Event::KEY_UP)
            {
                if (event.keyCode == 17)
                {
                    camera.keys.up = false;
                }
                if (event.keyCode == 31)
                {
                    camera.keys.down = false;
                }
                if (event.keyCode == 30)
                {
                    camera.keys.left = false;
                }
                if (event.keyCode == 32)
                {
                    camera.keys.right = false;
                }
            }
            auto anyKeyDown = event.type & Event::KEY_DOWN || event.type & Event::KEY_REPEAT;
            if (anyKeyDown)
            {
                if (event.keyCode == 17)
                {
                    camera.keys.up = true;
                }
                if (event.keyCode == 31)
                {
                    camera.keys.down = true;
                }
                if (event.keyCode == 30)
                {
                    camera.keys.left = true;
                }
                if (event.keyCode == 32)
                {
                    camera.keys.right = true;
                }
            }

            if (event.type == Event::MOUSE_MOVE)
            {

                if (mousePos.x == 0 && mousePos.y == 0)
                {
                    mousePos = {event.mouseX, event.mouseY};
                }
                int32_t dx = (int32_t)mousePos.x - event.mouseX;
                int32_t dy = (int32_t)mousePos.y - event.mouseY;

                camera.rotate(glm::vec3(dy * camera.rotationSpeed * deltaTime / 10.f, -dx * camera.rotationSpeed * deltaTime / 10.f, 0.0f));

                mousePos = {event.mouseX, event.mouseY};
            }
            camera.update(deltaTime / 100.f);
            uboVS.projectionMatrix = camera.matrices.perspective;
            uboVS.viewMatrix = camera.matrices.view;
            uboVS.modelMatrix = glm::mat4(1.0f);

            memcpy(uBuffer->Map(), &uboVS, sizeof(uboVS));
            uBuffer->Dirty(); });
        renderer->AddDrawState(rbs);

        engine->Frame();

        int di = 0;
    }

    return 0;
}