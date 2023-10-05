#pragma once

#include "EditorNode.h"

struct PipelineNode : public Node
{
    PipelineNode(const char *name, ImColor color = ImColor(255, 255, 255)) : Node(name, color)
    {
        this->Type = NodeType::PipelineNode;
        this->Inputs.emplace_back(Node::GetNextId(), "", PinType::Flow);
        this->Outputs.emplace_back(Node::GetNextId(), "", PinType::Flow);
        this->Outputs.emplace_back(Node::GetNextId(), "Attachments", PinType::Object);
        this->Inputs.emplace_back(Node::GetNextId(), "Descriptors", PinType::Object);
        this->Inputs.emplace_back(Node::GetNextId(), "Shaders", PinType::ShaderNodeIn);

        this->BuildNode();
    }

    virtual bool IsLinkValid(Pin *from, Pin *to) override
    {
        // connect from shaderNode
        if (from->Node->Type == NodeType::ShaderNode && from->Type == PinType::Delegate && to->Type == PinType::ShaderNodeIn)
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
        this->Outputs.emplace_back(Node::GetNextId(), "", PinType::Delegate);
        this->Inputs.emplace_back(Node::GetNextId(), "Vertex Shader", PinType::String);
        this->Inputs.emplace_back(Node::GetNextId(), "Fragment Shader", PinType::String);
        this->Inputs.emplace_back(Node::GetNextId(), "Compute Shader", PinType::String);

        this->BuildNode();
    }
};