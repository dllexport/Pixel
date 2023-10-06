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

    ed::NodeId contextNodeId = 0;

    ed::LinkId contextLinkId = 0;

    ed::PinId contextPinId = 0;

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

    Node *SpawnFilePathNode()
    {
        m_Nodes.push_back(new FilePathNode("", ImColor(255, 255, 128)));
        return m_Nodes.back().get();
    }

    Node *SpawnAttachmentTable()
    {
        m_Nodes.push_back(new AttachmentNode("Attachment Table", ImColor(255, 255, 128)));
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
                if (pin->ID == id)
                    return pin.get();

            for (auto &pin : node->Outputs)
                if (pin->ID == id)
                    return pin.get();
        }

        return nullptr;
    }
};