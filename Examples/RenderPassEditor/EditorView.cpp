#include "EditorView.h"

#include <imgui_node_editor_internal.h>

#include <FrameGraph/Graph.h>
#include <FrameGraph/GraphNodeJson.h>

#include <spdlog/spdlog.h>

static inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect &rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

ImGuiWindowFlags GetWindowFlags()
{
    return ImGuiWindowFlags_NoTitleBar |
           ImGuiWindowFlags_NoResize |
           ImGuiWindowFlags_NoMove |
           ImGuiWindowFlags_NoScrollbar |
           ImGuiWindowFlags_NoScrollWithMouse |
           ImGuiWindowFlags_NoSavedSettings |
           ImGuiWindowFlags_NoBringToFrontOnFocus;
}

void EditorView::BuildInputNode(IntrusivePtr<GraphNode> node, DescriptorTableNode *dtn)
{
    if (node->type == GraphNode::Type::BUFFER)
    {
        auto dn = static_cast<DescriptorGraphNode *>(node.get());

        auto mutableBufferNode = SpawnMutableBufferNode();
        mutableBufferNode->Name = dn->name;

        this->m_Links.push_back(Link(Node::GetNextId(), mutableBufferNode->LinkPin()->ID, dtn->BindingPin(dn->binding)->ID));
    }
}

void EditorView::BuildOutputNode(IntrusivePtr<GraphNode> node, AttachmentTableNode *dtn)
{
    if (node->type == GraphNode::Type::ATTACHMENT)
    {
        auto an = static_cast<AttachmentGraphNode *>(node.get());

        auto attachmentNode = SpawnTextureNode();
        attachmentNode->Name = an->name;
        attachmentNode->swapChain = an->swapChain;
        attachmentNode->depthStencil = an->depthStencil;

        this->m_Links.push_back(Link(Node::GetNextId(), dtn->BindingPin(0)->ID, attachmentNode->LinkPin()->ID));
    }
}

void EditorView::LoadJson()
{
    auto graph = Graph::ParseRenderPassJson("deferred.json");

    for (auto [level, nodes] : graph->Topo().levelsRenderPassOnly)
    {
        for (auto node : nodes)
        {
            if (node->type == GraphNode::Type::GRAPHIC_PASS)
            {
                auto gn = static_cast<GraphicRenderPassGraphNode *>(node);
                auto pipelineNode = SpawnPipelineNode();
                pipelineNode->Name = gn->name;

                auto shaderNode = SpawnShaderNode();
                FilePathNode *vertexFilePathNode = nullptr;
                FilePathNode *fragmentFilePathNode = nullptr;

                if (!gn->vertexShader.empty())
                {
                    vertexFilePathNode = SpawnFilePathNode();
                    vertexFilePathNode->SelectPin()->Name = gn->vertexShader;
                }

                if (!gn->framgmentShader.empty())
                {
                    fragmentFilePathNode = SpawnFilePathNode();
                    fragmentFilePathNode->SelectPin()->Name = gn->framgmentShader;
                }

                this->m_Links.push_back(Link(Node::GetNextId(), vertexFilePathNode->SelectPin()->ID, shaderNode->VertexShaderPin()->ID));
                this->m_Links.push_back(Link(Node::GetNextId(), fragmentFilePathNode->SelectPin()->ID, shaderNode->FragmentShaderPin()->ID));
                this->m_Links.push_back(Link(Node::GetNextId(), shaderNode->FlowOutPin()->ID, pipelineNode->ShaderPin()->ID));

                auto attachmentTableNode = SpawnAttachmentTableNode();
                auto descriptorTableNode = SpawnDescriptorTableNode();

                this->m_Links.push_back(Link(Node::GetNextId(), descriptorTableNode->LinkPin()->ID, pipelineNode->DescriptorPin()->ID));
                this->m_Links.push_back(Link(Node::GetNextId(), pipelineNode->AttachmentPin()->ID, attachmentTableNode->LinkOutPin()->ID));

                for (auto input : gn->inputs)
                {
                    BuildInputNode(input, descriptorTableNode);
                }

                for (auto output : gn->outputs)
                {
                    BuildOutputNode(output, attachmentTableNode);
                }
            }
        }
    }

    // for (auto [level, nodes] : graph->Topo().levels)
    // {
    //     for (auto node : nodes)
    //     {
    //         spdlog::info("{}, {}, {}", level, node->name, node->type);

    //         if (node->type == GraphNode::Type::GRAPHIC_PASS)
    //         {
    //             auto gn = static_cast<GraphicRenderPassGraphNode *>(node);
    //             auto pipelineNode = SpawnPipelineNode();
    //             pipelineNode->Name = gn->name;

    //             auto shaderNode = SpawnShaderNode();
    //             FilePathNode *vertexFilePathNode = nullptr;
    //             FilePathNode *fragmentFilePathNode = nullptr;

    //             if (!gn->vertexShader.empty())
    //             {
    //                 vertexFilePathNode = SpawnFilePathNode();
    //                 vertexFilePathNode->SelectPin()->Name = gn->vertexShader;
    //             }

    //             if (!gn->framgmentShader.empty())
    //             {
    //                 fragmentFilePathNode = SpawnFilePathNode();
    //                 fragmentFilePathNode->SelectPin()->Name = gn->framgmentShader;
    //             }

    //             this->m_Links.push_back(Link(Node::GetNextId(), vertexFilePathNode->SelectPin()->ID, shaderNode->VertexShaderPin()->ID));
    //             this->m_Links.push_back(Link(Node::GetNextId(), fragmentFilePathNode->SelectPin()->ID, shaderNode->FragmentShaderPin()->ID));
    //             this->m_Links.push_back(Link(Node::GetNextId(), shaderNode->FlowOutPin()->ID, pipelineNode->ShaderPin()->ID));
    //         }

    //         if (node->type == GraphNode::Type::ATTACHMENT)
    //         {
    //             auto an = static_cast<AttachmentGraphNode *>(node);
    //             auto attachmentTableNode = SpawnAttachmentTableNode();

    //             auto attachmentNode = SpawnAttachmentNode();
    //             attachmentNode->clear = an->clear;
    //             attachmentNode->swapchain = an->swapChain;
    //             attachmentNode->shared = an->swapChain;
    //             this->m_Links.push_back(Link(Node::GetNextId(), attachmentTableNode->BindingPin(0)->ID, attachmentNode->LinkPin()->ID));
    //         }

    //         if (node->type == GraphNode::Type::BUFFER)
    //         {
    //             auto dn = static_cast<DescriptorGraphNode *>(node);
    //             auto descriptorTableNode = SpawnDescriptorTableNode();

    //             auto mutableBufferNode = SpawnMutableBufferNode();
    //             mutableBufferNode->Name = dn->name;

    //             this->m_Links.push_back(Link(Node::GetNextId(), mutableBufferNode->LinkPin()->ID, descriptorTableNode->BindingPin(dn->binding)->ID));
    //         }
    //     }
    // }

    // for (auto subpassJson : graph->GetJson().subpasses)
    // {
    //     auto pipelineNode = SpawnPipelineNode();
    //     pipelineNode->Name = subpassJson.name;

    //     auto shaderNode = SpawnShaderNode();
    //     // ed::SetNodePosition(header_id, ImVec2(420, 20));

    //     FilePathNode *vertexFilePathNode = nullptr;
    //     FilePathNode *fragmentFilePathNode = nullptr;

    //     if (!subpassJson.shaders.vertex.empty())
    //     {
    //         vertexFilePathNode = SpawnFilePathNode();
    //         vertexFilePathNode->Outputs[0]->Name = subpassJson.shaders.vertex;
    //     }

    //     if (!subpassJson.shaders.fragment.empty())
    //     {
    //         fragmentFilePathNode = SpawnFilePathNode();
    //         fragmentFilePathNode->Outputs[0]->Name = subpassJson.shaders.fragment;
    //     }

    //     for (auto output : subpassJson.outputs)
    //     {
    //         if (output.type == "attachment")
    //         {
    //         }
    //     }
    // }
}

EditorView::EditorView(PixelEngine *engine) : ImguiOverlay(engine)
{
    LoadJson();
    Setup();
}

EditorView::~EditorView()
{
    if (m_Editor)
    {
        ed::DestroyEditor(m_Editor);
        m_Editor = nullptr;
    }
}

void EditorView::SetupFont()
{
    ImGuiIO &io = ImGui::GetIO();

    IM_DELETE(io.Fonts);

    io.Fonts = IM_NEW(ImFontAtlas);

    ImFontConfig config;
    config.OversampleH = 8;
    config.OversampleV = 8;
    config.PixelSnapH = false;

    io.Fonts->AddFontFromFileTTF("fonts/Play-Regular.ttf", 18.0f, &config);

    io.Fonts->Build();
}

void EditorView::Setup()
{
    SetupFont();
    ed::Config config;

    const float zoomLevels[] =
        {0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f};

    for (auto &level : zoomLevels)
        config.CustomZoomLevels.push_back(level);

    m_Editor = ed::CreateEditor(&config);
    ed::SetCurrentEditor(m_Editor);
}

void EditorView::ImGUINewFrame()
{
    if (!frameEnd)
        return;
    ImGui::NewFrame();
    frameEnd = false;

    auto &io = ImGui::GetIO();
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
            }
            if (ImGui::MenuItem("Save"))
            {
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetItemRectSize().y));
    auto canvaSize = io.DisplaySize;
    canvaSize.y -= ImGui::GetItemRectSize().y;
    ImGui::SetNextWindowSize(canvaSize);

    ImGui::Begin("Content", nullptr, GetWindowFlags());

    ed::SetCurrentEditor(m_Editor);

    Pin *newLinkPin = nullptr;
    bool createNewNode = false;
    Pin *newNodeLinkPin = nullptr;

    Node::DrawContext dc{
        .newLinkPin = newLinkPin,
        .m_Links = m_Links};

    ed::Begin("Node editor");
    {
        auto cursorTopLeft = ImGui::GetCursorScreenPos();

        util::BlueprintNodeBuilder builder;

        for (auto &node : m_Nodes)
        {
            node->Draw(dc);
        }

        for (auto &link : m_Links)
            ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

        if (!createNewNode)
        {
            if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
            {
                auto showLabel = [](const char *label, ImColor color)
                {
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
                    auto size = ImGui::CalcTextSize(label);

                    auto padding = ImGui::GetStyle().FramePadding;
                    auto spacing = ImGui::GetStyle().ItemSpacing;

                    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

                    auto rectMin = ImGui::GetCursorScreenPos() - padding;
                    auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

                    auto drawList = ImGui::GetWindowDrawList();
                    drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
                    ImGui::TextUnformatted(label);
                };

                ed::PinId startPinId = 0, endPinId = 0;
                if (ed::QueryNewLink(&startPinId, &endPinId))
                {
                    auto startPin = FindPin(startPinId);
                    auto endPin = FindPin(endPinId);

                    newLinkPin = startPin ? startPin : endPin;

                    if (startPin->Kind == PinKind::Input)
                    {
                        std::swap(startPin, endPin);
                        std::swap(startPinId, endPinId);
                    }

                    if (startPin && endPin)
                    {
                        if (endPin == startPin)
                        {
                            ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                        }
                        else if (endPin->Kind == startPin->Kind)
                        {
                            showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                            ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                        }
                        else if (endPin->Node == startPin->Node)
                        {
                            showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
                            ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                        }
                        else if (!endPin->Node->IsLinkValid(dc, startPin, endPin))
                        {
                            showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                            ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                        }
                        else if (IsLinkExist(startPinId, endPinId))
                        {
                            showLabel("x Link Already Exist", ImColor(45, 32, 32, 180));
                            ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                        }
                        else
                        {
                            showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                            if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                            {
                                m_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
                                m_Links.back().Color = Node::GetIconColor(startPin->Type);
                            }
                        }
                    }
                }

                ed::PinId pinId = 0;
                if (ed::QueryNewNode(&pinId))
                {
                    newLinkPin = FindPin(pinId);
                    if (newLinkPin)
                        showLabel("+ Create Node", ImColor(32, 45, 32, 180));

                    if (ed::AcceptNewItem())
                    {
                        createNewNode = true;
                        newNodeLinkPin = FindPin(pinId);
                        newLinkPin = nullptr;
                        ed::Suspend();
                        ImGui::OpenPopup("Create New Node");
                        ed::Resume();
                    }
                }
            }
            else
                newLinkPin = nullptr;

            ed::EndCreate();

            if (ed::BeginDelete())
            {
                ed::NodeId nodeId = 0;
                while (ed::QueryDeletedNode(&nodeId))
                {
                    if (ed::AcceptDeletedItem())
                    {
                        auto id = std::find_if(m_Nodes.begin(), m_Nodes.end(), [nodeId](auto &node)
                                               { return node->ID == nodeId; });
                        if (id != m_Nodes.end())
                            m_Nodes.erase(id);
                    }
                }

                ed::LinkId linkId = 0;
                while (ed::QueryDeletedLink(&linkId))
                {
                    if (ed::AcceptDeletedItem())
                    {
                        auto id = std::find_if(m_Links.begin(), m_Links.end(), [linkId](auto &link)
                                               { return link.ID == linkId; });
                        if (id != m_Links.end())
                            m_Links.erase(id);
                    }
                }
            }
            ed::EndDelete();
        }

        ImGui::SetCursorScreenPos(cursorTopLeft);
    }

    // popup menu
    auto openPopupPosition = ImGui::GetMousePos();
    ed::Suspend();
    if (ed::ShowNodeContextMenu(&contextNodeId))
        ImGui::OpenPopup("Node Context Menu");
    else if (ed::ShowPinContextMenu(&contextPinId))
        ImGui::OpenPopup("Pin Context Menu");
    else if (ed::ShowLinkContextMenu(&contextLinkId))
        ImGui::OpenPopup("Link Context Menu");
    else if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Create New Node");
        newNodeLinkPin = nullptr;
    }
    ed::Resume();

    ed::Suspend();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("Node Context Menu"))
    {
        auto node = FindNode(contextNodeId);

        ImGui::TextUnformatted("Node Context Menu");
        ImGui::Separator();
        if (node)
        {
            ImGui::Text("ID: %p", node->ID.AsPointer());
            ImGui::Text("Inputs: %d", (int)node->Inputs.size());
            ImGui::Text("Outputs: %d", (int)node->Outputs.size());
        }
        else
            ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteNode(contextNodeId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Pin Context Menu"))
    {
        auto pin = FindPin(contextPinId);

        ImGui::TextUnformatted("Pin Context Menu");
        ImGui::Separator();
        if (pin)
        {
            ImGui::Text("ID: %p", pin->ID.AsPointer());
            if (pin->Node)
                ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
            else
                ImGui::Text("Node: %s", "<none>");
        }
        else
            ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context Menu"))
    {
        auto link = FindLink(contextLinkId);

        ImGui::TextUnformatted("Link Context Menu");
        ImGui::Separator();
        if (link)
        {
            ImGui::Text("ID: %p", link->ID.AsPointer());
            ImGui::Text("From: %p", link->StartPinID.AsPointer());
            ImGui::Text("To: %p", link->EndPinID.AsPointer());
        }
        else
            ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteLink(contextLinkId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Create New Node"))
    {
        auto newNodePostion = openPopupPosition;
        // ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

        // auto drawList = ImGui::GetWindowDrawList();
        // drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

        Node *node = nullptr;
        if (ImGui::MenuItem("Pipeline Node"))
            node = SpawnPipelineNode();
        if (ImGui::MenuItem("Shader Node"))
            node = SpawnShaderNode();
        if (ImGui::MenuItem("FilePath Node"))
            node = SpawnFilePathNode();
        if (ImGui::MenuItem("Attachment Table Node"))
            node = SpawnAttachmentTableNode();
        if (ImGui::MenuItem("Attachment Node"))
            node = SpawnTextureNode();
        if (ImGui::MenuItem("Attachment Ref Node"))
            node = SpawnAttachmentReferenceNode();

        if (node)
        {
            createNewNode = false;

            ed::SetNodePosition(node->ID, newNodePostion);

            if (auto startPin = newNodeLinkPin)
            {
                auto &pins = startPin->Kind == PinKind::Input ? node->Outputs : node->Inputs;

                for (auto &pin : pins)
                {
                    if (Node::CanCreateLink(startPin, pin.get()))
                    {
                        auto endPin = &pin;
                        // if (startPin->Kind == PinKind::Input)
                        //     std::swap(startPin, endPin);

                        // m_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
                        // m_Links.back().Color = Node::GetIconColor(startPin->Type);

                        break;
                    }
                }
            }
        }

        ImGui::EndPopup();
    }
    else
        createNewNode = false;
    ImGui::PopStyleVar();
    ed::Resume();

    ed::End();

    if (firstFrame)
        ed::NavigateToContent(0.0f);

    ImGui::End();

    ImGui::Render();

    ImGui::EndFrame();

    ed::SetCurrentEditor(nullptr);
    firstFrame = false;
    frameEnd = true;
}
