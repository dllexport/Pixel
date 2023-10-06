#include <Engine/ImguiOverlay.h>

#include <imgui.h>
#include <imgui_internal.h>

ImguiOverlay::ImguiOverlay(PixelEngine *engine) : engine(engine)
{
    imguiContext = ImGui::CreateContext();
}

ImguiOverlay::~ImguiOverlay()
{
    ImGui::DestroyContext(imguiContext);
    // ensure there's no dangling drawState
    assert(drawState->use_count() == 1);
}

void ImguiOverlay::ImGUINewFrame()
{
    ImGui::NewFrame();
    
    ImGui::ShowDemoWindow();

    ImGui::Render();

    ImGui::EndFrame();
}

void ImguiOverlay::BuildPipeline()
{
    ColorBlendAttachmentState imguiColorBlendState = {
        .blendEnable = true,
        .srcColorBlendFactor = BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = BLEND_OP_ADD,
        .srcAlphaBlendFactor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .dstAlphaBlendFactor = BLEND_FACTOR_ZERO,
        .alphaBlendOp = BLEND_OP_ADD,
        .colorWriteMask = COLOR_COMPONENT_ALL_BIT};

    std::vector<VertexInputState> imguiInputStates{
        {0, 0, TextureFormat::FORMAT_R32G32_SFLOAT},
        {0, 1, TextureFormat::FORMAT_R32G32_SFLOAT},
        {0, 2, TextureFormat::FORMAT_R8G8B8A8_UNORM},
    };

    PipelineStates imguiPipelineStates = {
        .vertexInputStates = imguiInputStates,
        .inputAssembleState = {.type = InputAssembleState::Type::TRIANGLE_LIST},
        .rasterizationState = {.polygonMode = RasterizationState::PolygonModeType::FILL, .cullMode = RasterizationState::CullModeType::NONE, .frontFace = RasterizationState::FrontFaceType::COUNTER_CLOCKWISE, .lineWidth = 1.0f},
        .colorBlendAttachmentStates = {imguiColorBlendState},
        .depthStencilState = {.depthTestEnable = false, .depthWriteEnable = false}};

    imguiPipeline = engine->RegisterPipeline("singlePass", "imgui", imguiPipelineStates);
}

void ImguiOverlay::BuildDrawable()
{
    auto rhiRuntime = engine->GetRHIRuntime();
    auto imguiDrawable = rhiRuntime->CreateResourceBindingState(imguiPipeline);
    imguiDrawable->name = "imguiDrawable";

    ImGuiIO &io = ImGui::GetIO();

    io.DisplaySize = ImVec2(1024, 768);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    unsigned char *fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    auto uploadSize = texWidth * texHeight * 4 * sizeof(char);

    auto imguiFontHostBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, uploadSize);
    memcpy(imguiFontHostBuffer->Map(), fontData, uploadSize);

    auto imguiFontTexture = rhiRuntime->CreateTexture(TextureFormat::FORMAT_R8G8B8A8_UNORM, Texture::Usage::IMAGE_USAGE_SAMPLED_BIT | Texture::Usage::IMAGE_USAGE_TRANSFER_DST_BIT, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT, {uint32_t(texWidth), uint32_t(texHeight), 1});
    auto imguiFontSampler = rhiRuntime->CreateSampler(imguiFontTexture);
    imguiDrawable->Bind(0, 0, imguiFontSampler);

    engine->GetAuxiliaryExecutor()->TransferResource(imguiFontTexture, imguiFontHostBuffer);
    imguiFontHostBuffer.reset();

    auto imguiCBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_LOCAL_BIT, sizeof(ImguiPushConstant));
    auto pimguiCBuffer = (ImguiPushConstant *)imguiCBuffer->Map();
    pimguiCBuffer->scale = glm::vec2(2.0f / 1024.0f, 2.0f / 768.0f);
    pimguiCBuffer->translate = glm::vec2(-1.0f);
    imguiDrawable->Bind(imguiCBuffer);

    // capture IntrusivePtr
    auto imguiUpdateCallback = [this, rhiRuntime, imguiCBuffer, imguiDrawable](UpdateInput inputs)
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
            io.MouseDown[1] = event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_RIGHT;
            io.MouseDown[2] = event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_MIDDLE;
        }

        if (event.type & Event::MOUSE_UP)
        {
            io.MousePos = ImVec2(event.keyEvent.mouseX, event.keyEvent.mouseY);
            event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_LEFT ? io.MouseDown[0] = 0 : 0;
            event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_RIGHT ? io.MouseDown[1] = 0 : 0;
            event.keyEvent.keyCode == EVENT_MOUSE_BUTTON_MIDDLE ? io.MouseDown[2] = 0 : 0;
        }

        if (event.type == Event::MOUSE_SCROLL)
        {
            io.MouseWheel = event.scrollEvent.offsetY;
            io.MouseWheelH = event.scrollEvent.offsetX;
        }

        if (event.type == Event::CHARACTER)
        {
            io.AddInputCharacter(event.keyEvent.keyCode);
            return false;
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
            case EVENT_KEY_LEFT_ALT:
            {
                io.AddKeyEvent(ImGuiKey_ModAlt, keyDown);
                break;
            }
            default:
                break;
            }
#undef DECL_KEY
        }

        // draw ui to internal buffer
        ImGUINewFrame();

        auto &vertexBuffers = imguiDrawable->GetVertexBuffers();
        auto &indexBuffers = imguiDrawable->GetIndexBuffers();

        ImDrawData *imDrawData = ImGui::GetDrawData();
        // nullptr if draw nothing
        if (!imDrawData)
        {
            return false;
        }

        auto vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
        auto indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

        if ((vertexBufferSize == 0) || (indexBufferSize == 0))
        {
            return false;
        }

        // create buffer if not exist
        if (!vertexBuffers || !indexBuffers)
        {
            vertexBuffers = rhiRuntime->CreateMutableBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
            imguiDrawable->BindVertexBuffer(vertexBuffers);
            indexBuffers = rhiRuntime->CreateMutableBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
            imguiDrawable->BindIndexBuffer(indexBuffers, ResourceBindingState::INDEX_TYPE_UINT16);
        }

        auto mutableVertexBuffer = static_cast<MutableBuffer *>(vertexBuffers.get());
        auto mutableIndexBuffer = static_cast<MutableBuffer *>(indexBuffers.get());

        auto &vertexBuffer = mutableVertexBuffer->GetBuffer(inputs.currentImageIndex);
        auto &indexBuffer = mutableIndexBuffer->GetBuffer(inputs.currentImageIndex);

        // mutate buffer if change
        if (vertexBuffers && indexBuffers)
        {
            auto b1 = vertexBuffer->Size() != imDrawData->TotalVtxCount;
            if (b1)
            {
                auto vBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_VERTEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBufferSize);
                mutableVertexBuffer->SetBuffer(inputs.currentImageIndex, vBuffer);
            }

            auto b2 = indexBuffer->Size() != imDrawData->TotalIdxCount;
            if (b2)
            {
                auto iBuffer = rhiRuntime->CreateBuffer(Buffer::BUFFER_USAGE_INDEX_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBufferSize);
                mutableIndexBuffer->SetBuffer(inputs.currentImageIndex, iBuffer);
            }
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

        return io.WantCaptureMouse;
    };

    imguiDrawable->RegisterUpdateCallback({UI, imguiUpdateCallback});

    drawState = imguiDrawable;
}