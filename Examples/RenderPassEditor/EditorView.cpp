#include "EditorView.h"

#include <imgui_node_editor_internal.h>

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

ImColor GetIconColor(PinType type)
{
    switch (type)
    {
    default:
    case PinType::Flow:
        return ImColor(255, 255, 255);
    case PinType::Bool:
        return ImColor(220, 48, 48);
    case PinType::Int:
        return ImColor(68, 201, 156);
    case PinType::Float:
        return ImColor(147, 226, 74);
    case PinType::String:
        return ImColor(124, 21, 153);
    case PinType::Object:
        return ImColor(51, 150, 215);
    case PinType::Function:
        return ImColor(218, 0, 183);
    case PinType::Delegate:
        return ImColor(255, 48, 48);
    }
};

void DrawPinIcon(const Pin &pin, bool connected, int alpha)
{
    const int m_PinIconSize = 24;

    using ax::Widgets::IconType;

    IconType iconType;
    ImColor color = GetIconColor(pin.Type);
    color.Value.w = alpha / 255.0f;
    switch (pin.Type)
    {
    case PinType::Flow:
        iconType = IconType::Flow;
        break;
    case PinType::Bool:
        iconType = IconType::Circle;
        break;
    case PinType::Int:
        iconType = IconType::Circle;
        break;
    case PinType::Float:
        iconType = IconType::Circle;
        break;
    case PinType::String:
        iconType = IconType::Circle;
        break;
    case PinType::Object:
        iconType = IconType::Circle;
        break;
    case PinType::Function:
        iconType = IconType::Circle;
        break;
    case PinType::Delegate:
        iconType = IconType::Square;
        break;
    default:
        return;
    }

    ax::Widgets::Icon(ImVec2(static_cast<float>(m_PinIconSize), static_cast<float>(m_PinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
};

EditorView::EditorView(PixelEngine *engine) : ImguiOverlay(engine)
{
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

    io.Fonts->AddFontFromFileTTF("/Users/mario/Documents/GitHub/Pixel/Examples/RenderPassEditor/font/SF-Pro-Text-Regular.otf", 18.0f, &config);

    io.Fonts->Build();
}

void EditorView::Setup()
{
    SetupFont();
    ed::Config config;

    const float zoomLevels[] =
        {0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f};

    for (auto &level : zoomLevels)
        config.CustomZoomLevels.push_back(level);

    m_Editor = ed::CreateEditor(&config);
    ed::SetCurrentEditor(m_Editor);
    SpawnInputActionNode();

    BuildNodes();
}

void EditorView::ImGUINewFrame()
{
    ImGui::NewFrame();

    auto &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("Content", nullptr, GetWindowFlags());

    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    ed::SetCurrentEditor(m_Editor);

    Pin *newLinkPin = nullptr;
    bool createNewNode = false;
    Pin *newNodeLinkPin = nullptr;

    ed::Begin("Node editor");
    {
        auto cursorTopLeft = ImGui::GetCursorScreenPos();

        util::BlueprintNodeBuilder builder;

        for (auto &node : m_Nodes)
        {
            if (node.Type != NodeType::Blueprint && node.Type != NodeType::Simple)
                continue;

            const auto isSimple = node.Type == NodeType::Simple;

            bool hasOutputDelegates = false;
            for (auto &output : node.Outputs)
                if (output.Type == PinType::Delegate)
                    hasOutputDelegates = true;

            builder.Begin(node.ID);
            if (!isSimple)
            {
                builder.Header(node.Color);
                ImGui::Spring(0);
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::Spring(1);
                ImGui::Dummy(ImVec2(0, 28));
                if (hasOutputDelegates)
                {
                    ImGui::BeginVertical("delegates", ImVec2(0, 28));
                    ImGui::Spring(1, 0);
                    for (auto &output : node.Outputs)
                    {
                        if (output.Type != PinType::Delegate)
                            continue;

                        auto alpha = ImGui::GetStyle().Alpha;
                        if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                            alpha = alpha * (48.0f / 255.0f);

                        ed::BeginPin(output.ID, ed::PinKind::Output);
                        ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
                        ed::PinPivotSize(ImVec2(0, 0));
                        ImGui::BeginHorizontal(output.ID.AsPointer());
                        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                        if (!output.Name.empty())
                        {
                            ImGui::TextUnformatted(output.Name.c_str());
                            ImGui::Spring(0);
                        }
                        DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                        ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                        ImGui::EndHorizontal();
                        ImGui::PopStyleVar();
                        ed::EndPin();

                        // DrawItemRect(ImColor(255, 0, 0));
                    }
                    ImGui::Spring(1, 0);
                    ImGui::EndVertical();
                    ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                }
                else
                    ImGui::Spring(0);
                builder.EndHeader();
            }

            for (auto &input : node.Inputs)
            {
                auto alpha = ImGui::GetStyle().Alpha;
                if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
                    alpha = alpha * (48.0f / 255.0f);

                builder.Input(input.ID);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
                ImGui::Spring(0);
                if (!input.Name.empty())
                {
                    ImGui::TextUnformatted(input.Name.c_str());
                    ImGui::Spring(0);
                }
                if (input.Type == PinType::Bool)
                {
                    ImGui::Button("Hello");
                    ImGui::Spring(0);
                }
                ImGui::PopStyleVar();
                builder.EndInput();
            }

            if (isSimple)
            {
                builder.Middle();

                ImGui::Spring(1, 0);
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::Spring(1, 0);
            }

            for (auto &output : node.Outputs)
            {
                if (!isSimple && output.Type == PinType::Delegate)
                    continue;

                auto alpha = ImGui::GetStyle().Alpha;
                if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                    alpha = alpha * (48.0f / 255.0f);

                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                builder.Output(output.ID);
                if (output.Type == PinType::String)
                {
                    static char buffer[128] = "Edit Me\nMultiline!";
                    static bool wasActive = false;

                    ImGui::PushItemWidth(100.0f);
                    ImGui::InputText("##edit", buffer, 127);
                    ImGui::PopItemWidth();
                    if (ImGui::IsItemActive() && !wasActive)
                    {
                        ed::EnableShortcuts(false);
                        wasActive = true;
                    }
                    else if (!ImGui::IsItemActive() && wasActive)
                    {
                        ed::EnableShortcuts(true);
                        wasActive = false;
                    }
                    ImGui::Spring(0);
                }
                if (!output.Name.empty())
                {
                    ImGui::Spring(0);
                    ImGui::TextUnformatted(output.Name.c_str());
                }
                ImGui::Spring(0);
                DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                ImGui::PopStyleVar();
                builder.EndOutput();
            }

            builder.End();
        }

        for (auto &node : m_Nodes)
        {
            if (node.Type != NodeType::Comment)
                continue;

            const float commentAlpha = 0.75f;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
            ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
            ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
            ed::BeginNode(node.ID);
            ImGui::PushID(node.ID.AsPointer());
            ImGui::BeginVertical("content");
            ImGui::BeginHorizontal("horizontal");
            ImGui::Spring(1);
            ImGui::TextUnformatted(node.Name.c_str());
            ImGui::Spring(1);
            ImGui::EndHorizontal();
            ed::Group(node.Size);
            ImGui::EndVertical();
            ImGui::PopID();
            ed::EndNode();
            ed::PopStyleColor(2);
            ImGui::PopStyleVar();

            if (ed::BeginGroupHint(node.ID))
            {
                // auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
                auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

                // ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

                auto min = ed::GetGroupMin();
                // auto max = ed::GetGroupMax();

                ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
                ImGui::BeginGroup();
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::EndGroup();

                auto drawList = ed::GetHintBackgroundDrawList();

                auto hintBounds = ImGui_GetItemRect();
                auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

                drawList->AddRectFilled(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

                drawList->AddRect(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

                // ImGui::PopStyleVar();
            }
            ed::EndGroupHint();
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
                        // else if (endPin->Node == startPin->Node)
                        //{
                        //     showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
                        //     ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                        // }
                        else if (endPin->Type != startPin->Type)
                        {
                            showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                            ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                        }
                        else
                        {
                            showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                            if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                            {
                                m_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
                                m_Links.back().Color = GetIconColor(startPin->Type);
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
                                               { return node.ID == nodeId; });
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

    ed::End();

    if (firstFrame)
        ed::NavigateToContent(0.0f);

    ImGui::End();

    ImGui::Render();

    ImGui::EndFrame();

    ed::SetCurrentEditor(nullptr);
    firstFrame = false;
}
