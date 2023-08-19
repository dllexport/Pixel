#include <FrameGraph/Graph.h>
#include <RHI/RuntimeEntry.h>
#include <Engine/Renderer.h>
#include <spdlog/spdlog.h>
#include <Engine/Pixelengine.h>
#include <Engine/Renderable.h>

#include <Engine/Camera.h>
#include <glm/glm.hpp>

#include <imgui.h>

void ImGUINewFrame()
{
    ImGui::NewFrame();

    // SRS - ShowDemoWindow() sets its own initial position and size, cannot override here
    ImGui::ShowDemoWindow();

    // Render to generate draw buffers
    ImGui::Render();
}

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

struct ImguiPushConstant
{
    glm::vec2 scale;
    glm::vec2 translate;
};

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

        auto rhiRuntime = engine->GetRHIRuntime();

        auto renderer = engine->CreateRenderer();

        auto vBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
        memcpy(vBuffer->Map(), vertexBuffer.data(), vertexBufferSize);
        auto iBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
        memcpy(iBuffer->Map(), indexBuffer.data(), indexBufferSize);
        auto uBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uboVS));

        Camera camera;
        camera.type = Camera::CameraType::firstperson;
        camera.setPosition(glm::vec3(0.0f, 0.0f, -2.5f));
        camera.setRotation(glm::vec3(0.0f));
        camera.setPerspective(60.0f, (float)1024 / (float)768, 1.0f, 256.0f);
        glm::vec2 mousePos = {};

        uboVS.projectionMatrix = camera.matrices.perspective;
        uboVS.viewMatrix = camera.matrices.view;
        uboVS.modelMatrix = glm::mat4(1.0f);
        memcpy(uBuffer->Map(), &uboVS, sizeof(uboVS));

        auto rbs = rhiRuntime->CreateResourceBindingState(colorPipeline);
        rbs->Bind(0, 0, uBuffer);
        rbs->BindVertexBuffer(vBuffer);
        rbs->BindIndexBuffer(iBuffer, ResourceBindingState::INDEX_TYPE_UINT32);
        rbs->BindDrawOp({ResourceBindingState::DrawOP{
            .indexCount = 3,
            .instanceCount = 1,
            .firstIndex = 0,
            .vertexOffset = 0,
            .firstInstance = 1}});
        renderer->AddDrawState(rbs);

        // IMGUI
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

        io.DisplaySize = ImVec2(1024, 768);
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

        unsigned char *fontData;
        int texWidth, texHeight;
        io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
        auto uploadSize = texWidth * texHeight * 4 * sizeof(char);

        auto imguiFontTexture = rhiRuntime->CreateTexture(TextureFormat::FORMAT_B8G8R8A8_UNORM, Texture::Usage::IMAGE_USAGE_SAMPLED_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, {uint32_t(texWidth), uint32_t(texWidth), 1}, {});
        auto imguiDrawable = rhiRuntime->CreateResourceBindingState(imguiPipeline);
        renderer->AddDrawState(imguiDrawable);

        auto imguiCBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_LOCAL_BIT, sizeof(ImguiPushConstant));
        auto pimguiCBuffer = (ImguiPushConstant *)imguiCBuffer->Map();
        imguiDrawable->Bind(imguiCBuffer);

        // imgui update vertex index buffer every frame
        renderer->RegisterUpdateCallback(imguiDrawable, [&](Renderer::UpdateInputs inputs)
                                         {

            if (!(inputs.event.type & Event::FRAME)) {
                return;
            }

            // draw ui to internal buffer
            ImGUINewFrame();

            auto& vertexBuffer = inputs.rbs->GetVertexBuffer();
            auto& indexBuffer = inputs.rbs->GetIndexBuffer();
            
            ImDrawData* imDrawData = ImGui::GetDrawData();

            // Note: Alignment is done inside buffer creation
            auto vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
            auto indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

            if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
                return;
            }

            if ((!vertexBuffer) || (vertexBuffer->Size() != imDrawData->TotalVtxCount)) {
                auto vBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
                inputs.rbs->BindVertexBuffer(vBuffer);
            }

            if ((!indexBuffer) || (indexBuffer->Size() != imDrawData->TotalIdxCount)) {
                auto iBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
                inputs.rbs->BindIndexBuffer(iBuffer, ResourceBindingState::INDEX_TYPE_UINT16);
            }

            // Upload data
            ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer->Map();
            ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer->Map();

            for (int n = 0; n < imDrawData->CmdListsCount; n++) {
                const ImDrawList* cmd_list = imDrawData->CmdLists[n];
                memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                vtxDst += cmd_list->VtxBuffer.Size;
                idxDst += cmd_list->IdxBuffer.Size;
            } 

            int32_t vertexOffset = 0;
            int32_t indexOffset = 0;

            auto& drawOps = imguiDrawable->GetDrawOps();
            drawOps.clear();
            if (imDrawData->CmdListsCount > 0) {
                for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
                {
                    const ImDrawList* cmd_list = imDrawData->CmdLists[i];
                    for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
                    {
                        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
                        ResourceBindingState::DrawOP drawOp = {};
                        drawOp.scissorOffset = {std::max((int32_t)(pcmd->ClipRect.x), 0), std::max((int32_t)(pcmd->ClipRect.y), 0)};
                        drawOp.scissorExtent = {(uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x), (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y)};
                        drawOp.indexCount = pcmd->ElemCount;
                        drawOp.instanceCount = 1;
                        drawOp.firstIndex = indexOffset;
                        drawOp.vertexOffset = vertexOffset;
                        drawOp.firstInstance = 0;
                        drawOps.push_back(drawOp);

                        indexOffset += pcmd->ElemCount;
                    }
                    vertexOffset += cmd_list->VtxBuffer.Size;
                }
            } 
        });

        renderer->RegisterUpdateCallback(rbs, [&](Renderer::UpdateInputs inputs)
                                         {
            auto event = inputs.event;
            auto deltaTime = inputs.deltaTime;
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

        engine->Frame();

        int di = 0;
    }

    return 0;
}