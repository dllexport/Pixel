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
    void LoadJson();
    void BuildInputNode(IntrusivePtr<GraphNode> node, DescriptorTableNode *dtn);
    void BuildOutputNode(IntrusivePtr<GraphNode> node, AttachmentTableNode *dtn);

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

    PipelineNode *SpawnPipelineNode()
    {
        auto node = new PipelineNode("Pipeline 0", ImColor(255, 128, 128));
        m_Nodes.push_back(node);
        return node;
    }

    ShaderNode *SpawnShaderNode()
    {
        auto node = new ShaderNode("Shader Node", ImColor(255, 255, 128));
        m_Nodes.push_back(node);
        return node;
    }

    FilePathNode *SpawnFilePathNode()
    {
        auto node = new FilePathNode("", ImColor(255, 255, 128));
        m_Nodes.push_back(node);
        return node;
    }

    AttachmentTableNode *SpawnAttachmentTableNode()
    {
        auto node = new AttachmentTableNode("Attachment Table", ImColor(255, 255, 128));
        m_Nodes.push_back(node);
        return node;
    }

    TextureNode *SpawnTextureNode()
    {
        auto node = new TextureNode("Texture Node", ImColor(255, 255, 128));
        m_Nodes.push_back(node);
        return node;
    }

    DescriptorTableNode *SpawnDescriptorTableNode()
    {
        auto node = new DescriptorTableNode("Descriptor Table", ImColor(255, 255, 128));
        m_Nodes.push_back(node);
        return node;
    }

    MutableBufferNode *SpawnMutableBufferNode()
    {
        auto node = new MutableBufferNode("MutableBuffer", ImColor(255, 255, 128));
        m_Nodes.push_back(node);
        return node;
    }

    AttachmentReferenceNode *SpawnAttachmentReferenceNode()
    {
        auto node = new AttachmentReferenceNode("Attachment Ref", ImColor(255, 255, 128));
        m_Nodes.push_back(node);
        return node;
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