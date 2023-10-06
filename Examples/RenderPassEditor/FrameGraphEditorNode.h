#pragma once

#include "EditorNode.h"

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

struct AttachmentNode : public Node
{
    AttachmentNode(const char *name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::PipelineNode;
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

    // virtual void Draw(DrawContext &dc) override
    // {
    //     util::BlueprintNodeBuilder builder;

    //     auto node = this;

    //     auto &newLinkPin = dc.newLinkPin;

    //     const auto isSimple = node->Type == NodeType::AttachmentNode;

    //     builder.Begin(node->ID);

    //     DrawHeader(dc, builder, node);

    //     if (isSimple)
    //     {
    //         builder.Middle();

    //         ImGui::Spring(1, 0);
    //         ImGui::TextUnformatted(node->Name.c_str());
    //         ImGui::Spring(1, 0);
    //     }

    //     for (auto &output : node->Outputs)
    //     {
    //         if (!isSimple && output->Type == PinType::Delegate)
    //             continue;

    //         auto alpha = ImGui::GetStyle().Alpha;
    //         if (newLinkPin && !Node::CanCreateLink(newLinkPin, output.get()) && output.get() != newLinkPin)
    //             alpha = alpha * (48.0f / 255.0f);

    //         ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    //         builder.Output(output->ID);

    //         if (output->Type == PinType::StringButton)
    //         {
    //             ImGui::PushItemWidth(100.0f);

    //             if (ImGui::Button(output->Name.c_str()))
    //             {
    //                 if (output->auxCallback)
    //                     output->auxCallback();
    //             }
    //             ImGui::PopItemWidth();
    //             ImGui::Spring(0);
    //         }

    //         DrawPinIcon(*output, dc.IsPinLinked(output->ID), (int)(alpha * 255));
    //         ImGui::PopStyleVar();
    //         builder.EndOutput();
    //     }

    //     builder.End();
    // }
};