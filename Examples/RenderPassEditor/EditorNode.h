#pragma once

#include <string>

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
    Object,
    Function,
    Delegate,
    ShaderNodeIn
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
    ShaderNode
};

struct Node;

struct Pin
{
    ed::PinId ID;
    IntrusivePtr<Node> Node;
    std::string Name;
    PinType Type;
    PinKind Kind;

    Pin(int id, const char *name, PinType type) : ID(id), Node(nullptr), Name(name), Type(type), Kind(PinKind::Input)
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
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
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
            input.Node = this;
            input.Kind = PinKind::Input;
        }

        for (auto &output : this->Outputs)
        {
            output.Node = this;
            output.Kind = PinKind::Output;
        }
    }

    virtual bool IsLinkValid(Pin *from, Pin *to)
    {
        return false;
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

struct NodeIdLess
{
    bool operator()(const ed::NodeId &lhs, const ed::NodeId &rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};