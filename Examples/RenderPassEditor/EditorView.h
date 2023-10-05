#pragma once

#include <Engine/ImguiOverlay.h>

#include "FrameGraphEditorNode.h"

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
    std::vector<IntrusivePtr<Node>> m_Nodes;
    std::vector<Link> m_Links;
    int m_NextId = 1;
    const int m_PinIconSize = 24;

    bool firstFrame = true;

    int GetNextId()
    {
        return m_NextId++;
    }

    Node *SpawnInputActionNode()
    {
        m_Nodes.push_back(new PipelineNode("Pipeline 0", ImColor(255, 128, 128)));
        return m_Nodes.back().get();
    }

    Node *SpawnShaderNode()
    {
        m_Nodes.push_back(new ShaderNode("Shader Node", ImColor(255, 255, 128)));
        return m_Nodes.back().get();
    }

    Node *FindNode(ed::NodeId id)
    {
        for (auto &node : m_Nodes)
            if (node->ID == id)
                return node.get();

        return nullptr;
    }

    Link *FindLink(ed::LinkId id)
    {
        for (auto &link : m_Links)
            if (link.ID == id)
                return &link;

        return nullptr;
    }

    bool IsLinkExist(ed::PinId from, ed::PinId to)
    {
        for (auto &link : m_Links)
        {
            if (link.StartPinID == from && link.EndPinID == to)
            {
                return true;
            }
        }
        return false;
    }

    Pin *FindPin(ed::PinId id)
    {
        if (!id)
            return nullptr;

        for (auto &node : m_Nodes)
        {
            for (auto &pin : node->Inputs)
                if (pin.ID == id)
                    return &pin;

            for (auto &pin : node->Outputs)
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