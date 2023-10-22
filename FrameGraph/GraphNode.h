#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include <Core/IntrusivePtr.h>

#include <RHI/TextureFormat.h>

struct GraphNode : public IntrusiveUnsafeCounter<GraphNode>
{
    enum Type
    {
        REFERENCE,

        GRAPHIC_PASS,
        COMPUTE_PASS,

        ATTACHMENT,
        BUFFER,
        SAMPLER
    };

    explicit GraphNode(std::string name, Type type);

    void AddInput(GraphNode *from);

    void AddOutput(GraphNode *to);

    // get all outputs of outputs, filtered by type
    std::vector<IntrusivePtr<GraphNode>> TraceAllOutputs(Type type, uint32_t traceLevel);

    std::string name;
    std::string passName;
    Type type;

    friend class Graph;
    std::vector<IntrusivePtr<GraphNode>> inputs;
    std::vector<IntrusivePtr<GraphNode>> outputs;

    // subpasses use it as input (directly)
    std::unordered_set<std::string> inputSubPassNames;

    std::string GlobalName() {
        return passName + "::" + name;
    }
};

struct ResourceNode : public GraphNode
{
    explicit ResourceNode(std::string name, Type type) : GraphNode(name, type) {}
};

struct AttachmentGraphNode : public ResourceNode
{
    AttachmentGraphNode(std::string name, Type type) : ResourceNode(name, type) {}
    TextureFormat format = TextureFormat::FORMAT_NONE;
    bool depthStencil = false;
    bool swapChain = false;
    bool shared = false;
    bool color = false;
    bool clear = false;
};

struct DescriptorGraphNode : public ResourceNode
{
    DescriptorGraphNode(std::string name, Type type) : ResourceNode(name, type) {}
    bool sampler = false;
};

struct RenderPassGraphNode : public GraphNode
{
    using GraphNode::GraphNode;
    // node -> (set, binding)
    struct ResourceBindingPack
    {
        uint32_t set;
        uint32_t binding;
        Type type;
    };
    std::unordered_map<std::string, ResourceBindingPack> bindingSets;
};

struct GraphicRenderPassGraphNode : public RenderPassGraphNode
{
    GraphicRenderPassGraphNode(std::string name, Type type) : RenderPassGraphNode(name, type) {}
    std::string vertexShader;
    std::string framgmentShader;
};

struct ComputeRenderPassGraphNode : public RenderPassGraphNode
{
    ComputeRenderPassGraphNode(std::string name, Type type) : RenderPassGraphNode(name, type) {}
    std::string computeShader;
};