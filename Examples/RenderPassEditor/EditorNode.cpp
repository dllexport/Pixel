#include "EditorNode.h"

void Node::DrawHeader(DrawContext &dc, util::BlueprintNodeBuilder &builder, Node *node)
{
    bool hasOutputDelegates = false;
    for (auto &output : node->Outputs)
        if (output->Type == PinType::Delegate)
            hasOutputDelegates = true;

    auto newLinkPin = dc.newLinkPin;

    builder.Header(node->Color);
    ImGui::Spring(0);
    ImGui::TextUnformatted(node->Name.c_str());
    ImGui::Spring(1);
    ImGui::Dummy(ImVec2(0, 28));
    if (hasOutputDelegates)
    {
        ImGui::BeginVertical("delegates", ImVec2(0, 28));
        ImGui::Spring(1, 0);
        for (auto &output : node->Outputs)
        {
            if (output->Type != PinType::Delegate)
                continue;

            auto alpha = ImGui::GetStyle().Alpha;
            if (newLinkPin && !CanCreateLink(newLinkPin, output.get()) && output.get() != newLinkPin)
                alpha = alpha * (48.0f / 255.0f);

            ed::BeginPin(output->ID, ed::PinKind::Output);
            ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
            ed::PinPivotSize(ImVec2(0, 0));
            ImGui::BeginHorizontal(output->ID.AsPointer());
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            if (!output->Name.empty())
            {
                ImGui::TextUnformatted(output->Name.c_str());
                ImGui::Spring(0);
            }
            DrawPinIcon(*output, dc.IsPinLinked(output->ID), (int)(alpha * 255));
            ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
            ImGui::EndHorizontal();
            ImGui::PopStyleVar();
            ed::EndPin();
        }
        ImGui::Spring(1, 0);
        ImGui::EndVertical();
        ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
    }
    else
        ImGui::Spring(0);
    builder.EndHeader();
}

void Node::DrawInputs(DrawContext &dc, util::BlueprintNodeBuilder &builder, Node *node)
{
    auto &newLinkPin = dc.newLinkPin;

    for (auto &input : node->Inputs)
    {
        auto alpha = ImGui::GetStyle().Alpha;
        if (newLinkPin && !CanCreateLink(newLinkPin, input.get()) && input.get() != newLinkPin)
            alpha = alpha * (48.0f / 255.0f);

        builder.Input(input->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        DrawPinIcon(*input, dc.IsPinLinked(input->ID), (int)(alpha * 255));
        ImGui::Spring(0);
        if (!input->Name.empty())
        {
            ImGui::Text(input->Name.c_str());
            ImGui::Spring(0);
        }
        if (input->Type == PinType::Bool)
        {
            ImGui::Button("Hello");
            ImGui::Spring(0);
        }
        ImGui::PopStyleVar();
        builder.EndInput();
    }
}

void Node::DrawOutputs(DrawContext &dc, util::BlueprintNodeBuilder &builder, Node *node)
{
    auto &newLinkPin = dc.newLinkPin;

    const auto isSimple = node->Type == NodeType::Simple;

    for (auto &output : node->Outputs)
    {
        if (!isSimple && output->Type == PinType::Delegate)
            continue;

        auto alpha = ImGui::GetStyle().Alpha;
        if (newLinkPin && !CanCreateLink(newLinkPin, output.get()) && output.get() != newLinkPin)
            alpha = alpha * (48.0f / 255.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        builder.Output(output->ID);
        if (output->Type == PinType::String)
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

        if (output->Type == PinType::StringButton)
        {
            ImGui::PushItemWidth(100.0f);

            if (ImGui::Button(output->Name.c_str()))
            {
                if (output->auxCallback)
                    output->auxCallback();
            }
            ImGui::PopItemWidth();
            ImGui::Spring(0);
        }

        if (!output->Name.empty())
        {
            ImGui::Spring(0);
            ImGui::TextUnformatted(output->Name.c_str());
        }
        ImGui::Spring(0);
        DrawPinIcon(*output, dc.IsPinLinked(output->ID), (int)(alpha * 255));
        ImGui::PopStyleVar();
        builder.EndOutput();
    }
}

void Node::Draw(DrawContext &dc)
{
    util::BlueprintNodeBuilder builder;

    auto node = this;

    auto &newLinkPin = dc.newLinkPin;

    const auto isSimple = node->Type == NodeType::Simple;

    builder.Begin(node->ID);
    if (!isSimple)
    {
        DrawHeader(dc, builder, node);
    }

    DrawInputs(dc, builder, node);

    if (isSimple)
    {
        builder.Middle();

        ImGui::Spring(1, 0);
        ImGui::TextUnformatted(node->Name.c_str());
        ImGui::Spring(1, 0);
    }

    DrawOutputs(dc, builder, node);

    builder.End();
}