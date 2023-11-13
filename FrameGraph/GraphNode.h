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

    std::string passName;
    std::string groupName;
    Type type;

    friend class Graph;
    std::vector<IntrusivePtr<GraphNode>> inputs;
    std::vector<IntrusivePtr<GraphNode>> outputs;

    // subpasses use it as input (directly)
    std::unordered_set<std::string> inputSubPassNames;

    std::string LocalName()
    {
        return name;
    }

    std::string GlobalName()
    {
        if (name.starts_with("::"))
            return name;
        return groupName + "::" + passName + "::" + name;
    }

    // resource with same name is shared within group
    std::string ScopeName()
    {
        if (name.starts_with("::"))
            return name;
        return groupName + "::" + name;
    }

    std::string GroupName()
    {
        return groupName;
    }

    std::string PassName()
    {
        return passName;
    }

    template <class T>
    T As()
    {
        return static_cast<T>(this);
    }

private:
    std::string name;
};

struct ResourceNode : public GraphNode
{
    explicit ResourceNode(std::string name, Type type) : GraphNode(name, type) {}
    bool shared = false;

    uint8_t binding;

    bool internal;
    bool immutable;
};

struct AttachmentGraphNode : public ResourceNode
{
    AttachmentGraphNode(std::string name, Type type) : ResourceNode(name, type) {}
    TextureFormat format = TextureFormat::FORMAT_NONE;
    bool depthStencil = false;
    bool swapChain = false;
    bool color = false;
    bool clear = false;

    // TODO, resolve global state, check if resource is used as input
    bool input = false;
};

struct DescriptorGraphNode : public ResourceNode
{
    DescriptorGraphNode(std::string name, Type type) : ResourceNode(name, type) {}
    bool sampler = false;
    uint8_t set;
};

struct SSBOGraphNode : public DescriptorGraphNode
{
    SSBOGraphNode(std::string name, Type type) : DescriptorGraphNode(name, type) {}
    uint32_t size;
    TextureFormat format = TextureFormat::FORMAT_NONE;
};

struct RenderPassGraphNode : public GraphNode
{
    using GraphNode::GraphNode;

    // node (global name) -> (set, binding)
    struct ResourceBindingPack
    {
        uint32_t set;
        uint32_t binding;
        Type type;
    };
    std::unordered_map<IntrusivePtr<DescriptorGraphNode>, ResourceBindingPack> bindingSets;

    std::unordered_set<std::string> dependencies;
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