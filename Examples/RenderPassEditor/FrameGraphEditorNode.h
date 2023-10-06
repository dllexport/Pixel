#pragma once

#include "EditorNode.h"

#include <algorithm>
#include <vector>
#include <string>
#include <set>

#include "utilities/FileDialog.h"

struct PipelineNode : public Node
{
    PipelineNode(const char *name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::PipelineNode;
        this->Inputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));
        this->Outputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));
        this->Outputs.push_back(new Pin(Node::GetNextId(), "Attachments", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Descriptors", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Shaders", PinType::ShaderNodeIn));

        this->BuildNode();
    }

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to) override
    {
        if (dc.IsPinLinked(to->ID))
        {
            return false;
        }

        if (from->Node->Type == NodeType::PipelineNode && from->Type == PinType::Flow && to->Type == PinType::Flow)
        {
            return true;
        }
        // connect from shaderNode
        if (from->Node->Type == NodeType::ShaderNode && from->Type == PinType::Flow && to->Type == PinType::ShaderNodeIn)
        {
            return true;
        }

        return false;
    }
};

struct ShaderNode : public Node
{
    ShaderNode(const char *name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::ShaderNode;
        this->Outputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Vertex Shader", PinType::String));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Fragment Shader", PinType::String));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Compute Shader", PinType::String));

        this->BuildNode();
    }

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to) override
    {
        // reject if pin is already linked
        if (dc.IsPinLinked(to->ID))
        {
            return false;
        }

        // connect from shaderNode
        if (from->Node->Type == NodeType::Simple && from->Type == PinType::StringButton && to->Type == PinType::String)
        {
            return true;
        }

        return false;
    }
};

struct FilePathNode : public Node
{
    FilePathNode(const char *name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::Simple;
        IntrusivePtr<Pin> pin = new Pin(Node::GetNextId(), "Select", PinType::StringButton);
        pin->auxCallback = [pin]()
        {
            auto newPath = FileDialog::Open();
            if (!newPath.empty())
            {
                pin->Name = newPath;
            }
        };
        this->Outputs.emplace_back(pin);
        this->BuildNode();
    }

    virtual void Draw(DrawContext &dc) override
    {
        util::BlueprintNodeBuilder builder;

        auto node = this;

        auto &newLinkPin = dc.newLinkPin;

        const auto isSimple = node->Type == NodeType::Simple;

        builder.Begin(node->ID);

        if (isSimple)
        {
            builder.Middle();

            ImGui::Spring(1, 0);
            ImGui::TextUnformatted(node->Name.c_str());
            ImGui::Spring(1, 0);
        }

        for (auto &output : node->Outputs)
        {
            if (!isSimple && output->Type == PinType::Delegate)
                continue;

            auto alpha = ImGui::GetStyle().Alpha;
            if (newLinkPin && !Node::CanCreateLink(newLinkPin, output.get()) && output.get() != newLinkPin)
                alpha = alpha * (48.0f / 255.0f);

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
            builder.Output(output->ID);

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

            DrawPinIcon(*output, dc.IsPinLinked(output->ID), (int)(alpha * 255));
            ImGui::PopStyleVar();
            builder.EndOutput();
        }

        builder.End();
    }
};

struct AttachmentTableNode : public Node
{
    AttachmentTableNode(const char *name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::AttachmentTableNode;
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Link", PinType::PipelineNodeIn));
        this->Outputs.push_back(new Pin(Node::GetNextId(), "Binding 0", PinType::Object));
        this->Outputs.push_back(new Pin(Node::GetNextId(), "Binding 1", PinType::Object));
        this->Outputs.push_back(new Pin(Node::GetNextId(), "Binding 2", PinType::Object));
        this->Outputs.push_back(new Pin(Node::GetNextId(), "Binding 3", PinType::Object));

        this->BuildNode();
    }

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to) override
    {
        // connect from shaderNode
        if (from->Node->Type == NodeType::PipelineNode && from->Type == PinType::Object && to->Type == PinType::PipelineNodeIn)
        {
            return true;
        }

        return false;
    }
};

static std::string _labelPrefix(const char *const label)
{
    float width = ImGui::CalcItemWidth();

    float x = ImGui::GetCursorPosX();
    ImGui::Text(label);
    ImGui::SameLine();
    ImGui::SetCursorPosX(x + width * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::SetNextItemWidth(-1);

    std::string labelID = "##";
    labelID += label;

    return labelID;
}

struct AttachmentNode : public Node
{
    AttachmentNode(const char *name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::PipelineNode;
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Link", PinType::Object));
        this->BuildNode();
    }

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to) override
    {
        // connect from shaderNode
        if (from->Node->Type == NodeType::AttachmentTableNode && from->Type == PinType::Object && to->Type == PinType::Object)
        {
            return true;
        }

        return false;
    }

    virtual void Draw(DrawContext &dc)
    {
        util::BlueprintNodeBuilder builder;

        auto node = this;

        auto &newLinkPin = dc.newLinkPin;
        static bool do_popup = false;

        static std::set<std::string> formats = {
            "B8G8R8A8_UNORM",
            "B8G8R8A8_SRGB",
            "B8G8R8A8_FFFF"};

        static std::string popup_text = *formats.begin();

        builder.Begin(node->ID);

        DrawHeader(dc, builder, node);

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
            ImGui::PopStyleVar();
            builder.EndInput();
        }

        ImGui::PushItemWidth(96);

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Name");
        ImGui::SameLine();
        if (ImGui::Button("Node 0"))
        {
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Format");
        ImGui::SameLine();
        if (ImGui::Button(popup_text.c_str()))
        {
            do_popup = true; // Instead of saying OpenPopup() here, we set this bool, which is used later in the Deferred Pop-up Section
        }

        static bool check = false;
        ImGui::Checkbox("Clear", &check);
        ImGui::Checkbox("SwapChain", &check);
        ImGui::Checkbox("Shared", &check);

        builder.End();

        ed::Suspend();

        if (do_popup)
        {
            ImGui::OpenPopup("popup_button"); // Cause openpopup to stick open.
            do_popup = false;                 // disable bool so that if we click off the popup, it doesn't open the next frame.
        }

        if (ImGui::BeginPopup("popup_button"))
        {
            ImGui::PushItemWidth(200.0f);

            // Note: if it weren't for the child window, we would have to PushItemWidth() here to avoid a crash!
            static char str1[128] = "";
            std::set<std::string> filteredFormats = formats;
            ImGui::InputTextWithHint("##", "Search Formats", str1, IM_ARRAYSIZE(str1));

            auto textStr = std::string(str1);
            for (auto &format : formats)
            {
                if (format.find(textStr) == std::string::npos)
                {
                    filteredFormats.erase(format);
                }
            }

            for (auto &format : filteredFormats)
            {
                if (ImGui::Button(format.c_str(), ImVec2(-1.0f, 0.0f)))
                {
                    popup_text = format;
                    ImGui::CloseCurrentPopup(); // These calls revoke the popup open state, which was set by OpenPopup above.
                }
            }

            ImGui::PopItemWidth();

            ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
        }
        ed::Resume();
    }
};