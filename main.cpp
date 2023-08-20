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

// return update callback
auto createImguiDrawable(IntrusivePtr<RHIRuntime> rhiRuntime, IntrusivePtr<ResourceBindingState> imguiDrawable)
{
    // IMGUI
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.DisplaySize = ImVec2(1024, 768);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    unsigned char *fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    auto uploadSize = texWidth * texHeight * 4 * sizeof(char);

    Texture::Configuration config;
    config.tiling = Texture::IMAGE_TILING_LINEAR;
    auto imguiFontTexture = rhiRuntime->CreateTexture(TextureFormat::FORMAT_R8G8B8A8_UNORM, Texture::Usage::IMAGE_USAGE_SAMPLED_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, {uint32_t(texWidth), uint32_t(texHeight), 1}, config);
    auto imguiFontTextureMapped = imguiFontTexture->Map();
    memcpy(imguiFontTextureMapped, fontData, uploadSize);

    auto imguiFontSampler = rhiRuntime->CreateSampler(imguiFontTexture);
    imguiDrawable->Bind(0, 0, imguiFontSampler);

    auto imguiCBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_LOCAL_BIT, sizeof(ImguiPushConstant));
    auto pimguiCBuffer = (ImguiPushConstant *)imguiCBuffer->Map();
    pimguiCBuffer->scale = glm::vec2(2.0f / 1024.0f, 2.0f / 768.0f);
    pimguiCBuffer->translate = glm::vec2(-1.0f);
    imguiDrawable->Bind(imguiCBuffer);

    // capture IntrusivePtr
    return [rhiRuntime, imguiCBuffer, imguiDrawable](Renderer::UpdateInputs inputs)
    {
        auto &event = inputs.event;
        ImGuiIO &io = ImGui::GetIO();

        if (event.type == Event::RESIZE)
        {
            io.DisplaySize = ImVec2((float)event.resizeEvent.width, (float)event.resizeEvent.height);
            auto pimguiCBuffer = (ImguiPushConstant *)imguiCBuffer->Map();
            pimguiCBuffer->scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
            pimguiCBuffer->translate = glm::vec2(-1.0f);
        }
        if (event.type == Event::MOUSE_MOVE)
        {
            io.MousePos = ImVec2(event.keyEvent.mouseX, event.keyEvent.mouseY);
        }

        if (event.type & Event::MOUSE_DOWN)
        {
            io.MousePos = ImVec2(event.keyEvent.mouseX, event.keyEvent.mouseY);
            io.MouseDown[0] = event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_LEFT;
            io.MouseDown[1] = event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_MIDDLE;
            io.MouseDown[2] = event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_RIGHT;
        }

        if (event.type & Event::MOUSE_UP)
        {
            io.MousePos = ImVec2(event.keyEvent.mouseX, event.keyEvent.mouseY);
            event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_LEFT ? io.MouseDown[0] = 0 : 0;
            event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_MIDDLE ? io.MouseDown[1] = 0 : 0;
            event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_RIGHT ? io.MouseDown[2] = 0 : 0;
        }

        if (event.type & Event::KEY_DOWN || event.type & Event::KEY_REPEAT || event.type & Event::KEY_UP)
        {
            auto keyDown = event.type & Event::KEY_DOWN || event.type & Event::KEY_REPEAT;

#define DECL_KEY(key)                            \
    case EVENT_KEY_##key:                        \
    {                                            \
        io.AddKeyEvent(ImGuiKey_##key, keyDown); \
        break;                                   \
    }

            switch (event.keyEvent.keyCode)
            {
                DECL_KEY(Q)
                DECL_KEY(W)
                DECL_KEY(E)
                DECL_KEY(R)
                DECL_KEY(T)
                DECL_KEY(Y)
                DECL_KEY(U)
                DECL_KEY(I)
                DECL_KEY(O)
                DECL_KEY(P)
                DECL_KEY(A)
                DECL_KEY(S)
                DECL_KEY(D)
                DECL_KEY(F)
                DECL_KEY(G)
                DECL_KEY(H)
                DECL_KEY(J)
                DECL_KEY(K)
                DECL_KEY(L)
                DECL_KEY(Z)
                DECL_KEY(X)
                DECL_KEY(C)
                DECL_KEY(V)
                DECL_KEY(B)
                DECL_KEY(N)
                DECL_KEY(M)
                DECL_KEY(1)
                DECL_KEY(2)
                DECL_KEY(3)
                DECL_KEY(4)
                DECL_KEY(5)
                DECL_KEY(6)
                DECL_KEY(7)
                DECL_KEY(8)
                DECL_KEY(9)
                DECL_KEY(0)
                DECL_KEY(F1)
                DECL_KEY(F2)
                DECL_KEY(F3)
                DECL_KEY(F4)
                DECL_KEY(F5)
                DECL_KEY(F6)
                DECL_KEY(F7)
                DECL_KEY(F8)
                DECL_KEY(F9)
                DECL_KEY(F10)
                DECL_KEY(F11)
                DECL_KEY(F12)
            default:
                break;
            }
#undef DECL_KEY
        }

        // draw ui to internal buffer
        ImGUINewFrame();

        auto &vertexBuffer = inputs.rbs->GetVertexBuffer();
        auto &indexBuffer = inputs.rbs->GetIndexBuffer();

        ImDrawData *imDrawData = ImGui::GetDrawData();

        auto vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        auto indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        if ((vertexBufferSize == 0) || (indexBufferSize == 0))
        {
            return;
        }

        if ((!vertexBuffer) || (vertexBuffer->Size() != imDrawData->TotalVtxCount))
        {
            auto vBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
            inputs.rbs->BindVertexBuffer(vBuffer);
        }

        if ((!indexBuffer) || (indexBuffer->Size() != imDrawData->TotalIdxCount))
        {
            auto iBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
            inputs.rbs->BindIndexBuffer(iBuffer, ResourceBindingState::INDEX_TYPE_UINT16);
        }

        // Upload data
        ImDrawVert *vtxDst = (ImDrawVert *)vertexBuffer->Map();
        ImDrawIdx *idxDst = (ImDrawIdx *)indexBuffer->Map();

        for (int n = 0; n < imDrawData->CmdListsCount; n++)
        {
            const ImDrawList *cmd_list = imDrawData->CmdLists[n];
            memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtxDst += cmd_list->VtxBuffer.Size;
            idxDst += cmd_list->IdxBuffer.Size;
        }

        int32_t vertexOffset = 0;
        int32_t indexOffset = 0;

        auto &drawOps = imguiDrawable->GetDrawOps();
        drawOps.clear();
        if (imDrawData->CmdListsCount > 0)
        {
            for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
            {
                const ImDrawList *cmd_list = imDrawData->CmdLists[i];
                for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
                {
                    const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[j];
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
    };
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    {
        auto graph = Graph::ParseRenderPassJson("C:/Users/Mario/Desktop/Pixel/simple.json");
        PipelineStates colorPipelineStates = {
            .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
            .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
            .depthStencilState = {.depthTestEnable = true, .depthWriteEnable = true}};

        ColorBlendAttachmentState imguiColorBlendState = {
            .blendEnable = true,
            .srcColorBlendFactor = BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = BLEND_OP_ADD,
            .srcAlphaBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .dstAlphaBlendFactor = BLEND_FACTOR_ZERO,
            .alphaBlendOp = BLEND_OP_ADD,
            .colorWriteMask = COLOR_COMPONENT_ALL_BIT};

        PipelineStates imguiPipelineStates = {
            .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
            .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
            .colorBlendAttachmentStates = {imguiColorBlendState},
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

        renderer->RegisterUpdateCallback(rbs, [&](Renderer::UpdateInputs inputs)
                                         {
            auto event = inputs.event;
            auto deltaTime = inputs.deltaTime;
            if (event.type == Event::KEY_UP)
            {
                if (event.keyEvent.keyCode == EVENT_KEY_W)
                    camera.keys.up = false;
                if (event.keyEvent.keyCode == EVENT_KEY_A)
                    camera.keys.left = false;
                if (event.keyEvent.keyCode == EVENT_KEY_S)
                    camera.keys.down = false;
                if (event.keyEvent.keyCode == EVENT_KEY_D)
                    camera.keys.right = false;
            }
            auto anyKeyDown = event.type & Event::KEY_DOWN || event.type & Event::KEY_REPEAT;
            if (anyKeyDown)
            {
                if (event.keyEvent.keyCode == EVENT_KEY_W)
                    camera.keys.up = true;
                if (event.keyEvent.keyCode == EVENT_KEY_A)
                    camera.keys.left = true;
                if (event.keyEvent.keyCode == EVENT_KEY_S)
                    camera.keys.down = true;
                if (event.keyEvent.keyCode == EVENT_KEY_D)
                    camera.keys.right = true;
            }

            if (event.type == Event::MOUSE_MOVE)
            {
                if (inputs.ioState.mouse[0]) {
                    if (mousePos.x == 0 && mousePos.y == 0)
                    {
                        mousePos = {event.keyEvent.mouseX, event.keyEvent.mouseY};
                    }
                    int32_t dx = (int32_t)mousePos.x - event.keyEvent.mouseX;
                    int32_t dy = (int32_t)mousePos.y - event.keyEvent.mouseY;

                    camera.rotate(glm::vec3(dy * camera.rotationSpeed * deltaTime / 10.f, -dx * camera.rotationSpeed * deltaTime / 10.f, 0.0f));

                    mousePos = {event.keyEvent.mouseX, event.keyEvent.mouseY};
                }
            }

            if (event.type == Event::MOUSE_UP) {
                mousePos = {};
            }

            camera.update(deltaTime / 100.f);
            uboVS.projectionMatrix = camera.matrices.perspective;
            uboVS.viewMatrix = camera.matrices.view;
            uboVS.modelMatrix = glm::mat4(1.0f);

            memcpy(uBuffer->Map(), &uboVS, sizeof(uboVS));
            uBuffer->Dirty(); });

        auto imguiDrawable = rhiRuntime->CreateResourceBindingState(imguiPipeline);
        renderer->AddDrawState(imguiDrawable);
        auto imguiUpdateCallback = createImguiDrawable(rhiRuntime, imguiDrawable);
        renderer->RegisterUpdateCallback(imguiDrawable, imguiUpdateCallback);

        engine->Frame();

        int di = 0;
    }

    return 0;
}