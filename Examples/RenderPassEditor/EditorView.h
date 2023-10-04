#pragma once

#include <Engine/ImguiOverlay.h>

#include "EditorNode.h"

class EditorView : public ImguiOverlay
{
public:
    EditorView(PixelEngine *engine);
    virtual ~EditorView();
    virtual void ImGUINewFrame() override;

    void Setup();
    void SetupFont();

private:
    ax::NodeEditor::EditorContext *m_Editor = nullptr;
    std::vector<Node> m_Nodes;
    std::vector<Link> m_Links;
    int m_NextId = 1;
    const int m_PinIconSize = 24;

    bool firstFrame = true;

    int GetNextId()
    {
        return m_NextId++;
    }

    void BuildNode(Node *node)
    {
        for (auto &input : node->Inputs)
        {
            input.Node = node;
            input.Kind = PinKind::Input;
        }

        for (auto &output : node->Outputs)
        {
            output.Node = node;
            output.Kind = PinKind::Output;
        }
    }

    void BuildNodes()
    {
        for (auto &node : m_Nodes)
            BuildNode(&node);
    }

    Node *SpawnInputActionNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Pipeline 0", ImColor(255, 128, 128));
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "Attachments", PinType::Object);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Descriptors", PinType::Object);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Shaders", PinType::Object);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node *SpawnShaderNode()
    {
        m_Nodes.emplace_back(GetNextId(), "Shader Node", ImColor(255, 255, 128));
        m_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Vertex Shader", PinType::String);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Fragment Shader", PinType::String);
        m_Nodes.back().Inputs.emplace_back(GetNextId(), "Compute Shader", PinType::String);

        BuildNode(&m_Nodes.back());

        return &m_Nodes.back();
    }

    Node *FindNode(ed::NodeId id)
    {
        for (auto &node : m_Nodes)
            if (node.ID == id)
                return &node;

        return nullptr;
    }

    Link *FindLink(ed::LinkId id)
    {
        for (auto &link : m_Links)
            if (link.ID == id)
                return &link;

        return nullptr;
    }

    Pin *FindPin(ed::PinId id)
    {
        if (!id)
            return nullptr;

        for (auto &node : m_Nodes)
        {
            for (auto &pin : node.Inputs)
                if (pin.ID == id)
                    return &pin;

            for (auto &pin : node.Outputs)
                if (pin.ID == id)
                    return &pin;
        }

        return nullptr;
    }

    bool IsPinLinked(ed::PinId id)
    {
        if (!id)
            return false;

        for (auto &link : m_Links)
            if (link.StartPinID == id || link.EndPinID == id)
                return true;

        return false;
    }

    bool CanCreateLink(Pin *a, Pin *b)
    {
        if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
            return false;

        return true;
    }
};