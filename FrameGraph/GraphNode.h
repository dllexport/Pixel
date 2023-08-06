#pragma once

#include <string>
#include <vector>

#include <Core/IntrusivePtr.h>

#include <RHI/TextureFormat.h>

struct GraphNode : public IntrusiveUnsafeCounter<GraphNode>
{
    enum Type {
        REFERENCE,

        GRAPHIC_PASS,
        COMPUTE_PASS,

        ATTACHMENT,
        BUFFER
    };

    explicit GraphNode(std::string name, Type type) : name(name), type(type) {}
    std::string name;
    Type type;
    uint16_t inputDegree = 0;

    void AddInput(GraphNode *from)
    {
        inputs.push_back(from);
        from->outputs.push_back(this);
    }

    void AddOutput(GraphNode *to)
    {
        outputs.push_back(to);
        to->inputs.push_back(this);
    }

    friend class Graph;
    std::vector<IntrusivePtr<GraphNode>> inputs;
    std::vector<IntrusivePtr<GraphNode>> outputs;
};

struct AttachmentGraphNode : public GraphNode
{
    AttachmentGraphNode(std::string name, Type type) : GraphNode(name, type) {}
    TextureFormat format = TextureFormat::NONE;
    bool depthStencil = false;
    bool input = false;
    bool color = false;
};

struct DescriptorGraphNode : public GraphNode
{
    DescriptorGraphNode(std::string name, Type type) : GraphNode(name, type) {}
};

struct GraphicRenderPassGraphNode : public GraphNode
{
    GraphicRenderPassGraphNode(std::string name, Type type) : GraphNode(name, type) {}
};

struct ComputeRenderPassGraphNode : public GraphNode
{
    ComputeRenderPassGraphNode(std::string name, Type type) : GraphNode(name, type) {}
};