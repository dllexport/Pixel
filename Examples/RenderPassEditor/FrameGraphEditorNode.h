#pragma once

#include "EditorNode.h"

#include <algorithm>
#include <vector>
#include <string>
#include <set>

#include "utilities/FileDialog.h"

struct PipelineNode : public Node
{
    PipelineNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::PipelineNode;
        this->Inputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Descriptors", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Shaders", PinType::ShaderNodeIn));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Attachments", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "AttachmentsRefs", PinType::Object));

        this->Outputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));

        this->BuildNode();
    }

    Pin *FlowInPin()
    {
        return Inputs[0].get();
    }

    Pin *FlowOutPin()
    {
        return Outputs[0].get();
    }

    Pin *AttachmentPin()
    {
        return Inputs[3].get();
    }

    Pin *DescriptorPin()
    {
        return Inputs[1].get();
    }

    Pin *ShaderPin()
    {
        return Inputs[2].get();
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

        // connect from descriptor table
        if (from->Node->Type == NodeType::DescriptorTableNode && from->Type == PinType::Flow && to->Type == PinType::Object)
        {
            return true;
        }

        return false;
    }
};

struct ShaderNode : public Node
{
    ShaderNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::ShaderNode;
        this->Outputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Vertex Shader", PinType::String));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Fragment Shader", PinType::String));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Compute Shader", PinType::String));

        this->BuildNode();
    }

    Pin *FlowOutPin()
    {
        return Outputs[0].get();
    }

    Pin *VertexShaderPin()
    {
        return Inputs[0].get();
    }

    Pin *FragmentShaderPin()
    {
        return Inputs[1].get();
    }

    Pin *ComputerShaderPin()
    {
        return Inputs[2].get();
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
    FilePathNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
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

    Pin *SelectPin()
    {
        return Outputs[0].get();
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
    AttachmentTableNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::AttachmentTableNode;
        this->Outputs.push_back(new Pin(Node::GetNextId(), "Link", PinType::PipelineNodeIn));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding 0", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding 1", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding 2", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding 3", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding 4", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding 4", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding 4", PinType::Object));
        this->BuildNode();
    }

    Pin *LinkOutPin()
    {
        return Outputs[0].get();
    }

    Pin *BindingPin(uint32_t binding)
    {
        static uint32_t auxCounter = 0;
        return Inputs[auxCounter++].get();
    }

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to) override
    {
        // connect from shaderNode
        if (from->Node->Type == NodeType::PipelineNode && from->Type == PinType::Object && to->Type == PinType::PipelineNodeIn)
        {
            return true;
        }

        if (from->Node->Type == NodeType::PipelineNode && from->Type == PinType::Object && to->Type == PinType::Object)
        {
            return true;
        }

        return false;
    }
};

struct TextureNode : public Node
{
    TextureNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::AttachmentNode;
        this->Outputs.push_back(new Pin(Node::GetNextId(), "Link", PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Link", PinType::Object));
        this->BuildNode();
    }

    bool shared = false;
    bool swapChain = false;
    bool depthStencil = false;

    Pin *LinkPin()
    {
        return Outputs[0].get();
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

        DrawInputs(dc, builder, node);
        DrawOutputs(dc, builder, node);

        ImGui::BeginHorizontal("example_h12", ImVec2(0, 0));
        ImGui::TextUnformatted("Name:");
        ImGui::Spring(1);
        if (ImGui::Button(Name.c_str()))
        {
        }
        ImGui::EndHorizontal();

        ImGui::BeginHorizontal("example_h2", ImVec2(0, 0));
        ImGui::TextUnformatted("Format:");
        ImGui::Spring(1);
        if (ImGui::Button(popup_text.c_str()))
        {
            do_popup = true; // Instead of saying OpenPopup() here, we set this bool, which is used later in the Deferred Pop-up Section
        }
        ImGui::EndHorizontal();

        ImGui::Checkbox("Shared", &shared);
        ImGui::Checkbox("SwapChain", &swapChain);
        ImGui::Checkbox("DepthStencil", &depthStencil);
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

            ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
        }
        ed::Resume();
    }
};

struct DescriptorTableNode : public Node
{
    DescriptorTableNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::DescriptorTableNode;
        this->Outputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding " + std::to_string(this->Inputs.size()), PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding " + std::to_string(this->Inputs.size()), PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding " + std::to_string(this->Inputs.size()), PinType::Object));
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding " + std::to_string(this->Inputs.size()), PinType::Object));

        this->BuildNode();
    }

    int bindingSize = 4;
    bool swapchain = false;
    bool clear = false;

    Pin *LinkPin()
    {
        return Outputs[0].get();
    }

    Pin *BindingPin(int binding)
    {
        return Inputs[binding].get();
    }

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to) override
    {
        // connect from shaderNode
        if (from->Node->Type == NodeType::MutableBuffer && from->Type == PinType::Flow && to->Type == PinType::Object)
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

        builder.Begin(node->ID);

        DrawHeader(dc, builder, node);

        DrawInputs(dc, builder, node);

        if (Inputs.size() < bindingSize)
        {
            this->Inputs.push_back(new Pin(Node::GetNextId(), "Binding " + std::to_string(this->Inputs.size()), PinType::Object));
            BuildNode();
        }

        DrawOutputs(dc, builder, node);

        builder.End();

        ed::Suspend();

        ed::Resume();
    }
};

struct MutableBufferNode : public Node
{
    MutableBufferNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::MutableBuffer;
        this->Outputs.push_back(new Pin(Node::GetNextId(), "", PinType::Flow));

        this->BuildNode();
    }

    Pin *LinkPin()
    {
        return Outputs[0].get();
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

        builder.Begin(node->ID);

        DrawHeader(dc, builder, node);

        ImGui::BeginVertical("id2", ImVec2(0, 28));
        ImGui::BeginHorizontal("BF1", ImVec2(0, 0));
        ImGui::TextUnformatted("Name:");
        ImGui::Spring(1);
        if (ImGui::Button("Buffer"))
        {
        }
        ImGui::EndHorizontal();

        ImGui::EndVertical();

        DrawOutputs(dc, builder, node);

        builder.End();

        ed::Suspend();

        ed::Resume();
    }
};

struct AttachmentReferenceNode : public Node
{
    AttachmentReferenceNode(std::string name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::PipelineNode;
        this->Inputs.push_back(new Pin(Node::GetNextId(), "Link", PinType::Object));
        this->BuildNode();
    }

    bool clear = false;

    Pin *LinkPin()
    {
        return Inputs[0].get();
    }

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to) override
    {
        // connect from shaderNode
        if (from->Node->Type == NodeType::AttachmentTableNode && from->Type == PinType::Object && to->Type == PinType::Object)
        {
            return true;
        }

        if (from->Node->Type == NodeType::PipelineNode && from->Type == PinType::Object && to->Type == PinType::Object)
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
            "Binding 1 +",
            "Binding 2 +",
            "Binding 3 +"};

        static std::string popup_text = *formats.begin();

        builder.Begin(node->ID);

        DrawHeader(dc, builder, node);

        DrawInputs(dc, builder, node);
        DrawOutputs(dc, builder, node);

        ImGui::BeginHorizontal("example_h22", ImVec2(0, 0));
        ImGui::TextUnformatted("Binding:");
        ImGui::Spring(1);
        if (ImGui::Button(popup_text.c_str()))
        {
            do_popup = true; // Instead of saying OpenPopup() here, we set this bool, which is used later in the Deferred Pop-up Section
        }
        ImGui::EndHorizontal();

        ImGui::Checkbox("Clear", &clear);

        builder.End();

        ed::Suspend();

        if (do_popup)
        {
            ImGui::OpenPopup("popup_button2"); // Cause openpopup to stick open.
            do_popup = false;                  // disable bool so that if we click off the popup, it doesn't open the next frame.
        }

        if (ImGui::BeginPopup("popup_button2"))
        {
            ImGui::PushItemWidth(200.0f);

            // Note: if it weren't for the child window, we would have to PushItemWidth() here to avoid a crash!
            static char str1[128] = "";
            std::set<std::string> filteredFormats = formats;

            for (auto &format : formats)
            {
                if (ImGui::Button(format.c_str()))
                {
                    popup_text = format;
                    ImGui::CloseCurrentPopup(); // These calls revoke the popup open state, which was set by OpenPopup above.
                }
            }

            ImGui::EndPopup(); // Note this does not do anything to the popup open/close state. It just terminates the content declaration.
        }
        ed::Resume();
    }
};