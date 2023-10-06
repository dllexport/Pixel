#pragma once

#include <string>
#include <vector>

#include <Core/IntrusivePtr.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#undef IMGUI_DEFINE_MATH_OPERATORS

#include <imgui_node_editor.h>

#include "utilities/builders.h"
#include "utilities/widgets.h"

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    StringButton,
    Object,
    Function,
    Delegate,
    ShaderNodeIn,
    PipelineNodeIn
};

enum class PinKind
{
    Output,
    Input
};

enum class NodeType
{
    Blueprint,
    Simple,
    Tree,
    Comment,
    Houdini,
    PipelineNode,
    ShaderNode,
    AttachmentNode
};

struct Node;

struct Pin : public IntrusiveCounter<Pin>
{
    ed::PinId ID;
    IntrusivePtr<Node> Node;
    std::string Name;
    PinType Type;
    PinKind Kind;

    std::function<void()> auxCallback;

    Pin(int id, const char *name, PinType type) : ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
    {
    }
};

struct Link
{
    ed::LinkId ID;

    ed::PinId StartPinID;
    ed::PinId EndPinID;

    ImColor Color;

    Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId) : ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

struct Node : public IntrusiveCounter<Node>
{
    inline static int m_NextId = 1;

    static int GetNextId()
    {
        return m_NextId++;
    }

    ed::NodeId ID;
    std::string Name;
    std::vector<IntrusivePtr<Pin>> Inputs;
    std::vector<IntrusivePtr<Pin>> Outputs;
    ImColor Color;
    NodeType Type;
    ImVec2 Size;

    std::string State;
    std::string SavedState;

    Node(const char *name, ImColor color = ImColor(255, 255, 255)) : ID(GetNextId()), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
    {
    }

    void BuildNode()
    {
        for (auto &input : this->Inputs)
        {
            input->Node = this;
            input->Kind = PinKind::Input;
        }

        for (auto &output : this->Outputs)
        {
            output->Node = this;
            output->Kind = PinKind::Output;
        }
    }

    struct DrawContext
    {
        Pin *newLinkPin;
        std::vector<Link> &m_Links;

        bool IsPinLinked(ed::PinId id)
        {
            if (!id)
                return false;

            for (auto &link : m_Links)
                if (link.StartPinID == id || link.EndPinID == id)
                    return true;

            return false;
        }
    };

    virtual void Draw(DrawContext &dc);
    virtual void DrawHeader(DrawContext &dc, util::BlueprintNodeBuilder &builder, Node *node);

    virtual bool IsLinkValid(DrawContext &dc, Pin *from, Pin *to)
    {
        return false;
    }

    static ImColor GetIconColor(PinType type)
    {
        switch (type)
        {
        default:
            return ImColor(255, 255, 255);
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

    static void DrawPinIcon(const Pin &pin, bool connected, int alpha)
    {
        const int m_PinIconSize = 24;

        using ax::Widgets::IconType;

        IconType iconType;
        ImColor color = Node::GetIconColor(pin.Type);
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
        case PinType::PipelineNodeIn:
            iconType = IconType::Grid;
            break;
        default:
            iconType = IconType::Circle;
            break;
        }

        ax::Widgets::Icon(ImVec2(static_cast<float>(m_PinIconSize), static_cast<float>(m_PinIconSize)), iconType, connected, color, ImColor(32, 32, 32, alpha));
    };

    static bool CanCreateLink(Pin *a, Pin *b)
    {
        if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
            return false;

        return true;
    }
};

struct NodeIdLess
{
    bool operator()(const ed::NodeId &lhs, const ed::NodeId &rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};