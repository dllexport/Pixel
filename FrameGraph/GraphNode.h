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
    Type type;
    uint16_t inputDegree = 0;

    friend class Graph;
    std::vector<IntrusivePtr<GraphNode>> inputs;
    std::vector<IntrusivePtr<GraphNode>> outputs;

    // subpasses use it as input (directly)
    std::unordered_set<std::string> inputSubPassNames;
};

struct ResourceNode : public GraphNode
{
    explicit ResourceNode(std::string name, Type type) : GraphNode(name, type) {}

    uint32_t set = 0;
    uint32_t binding = UINT32_MAX;
};

struct AttachmentGraphNode : public ResourceNode
{
    AttachmentGraphNode(std::string name, Type type) : ResourceNode(name, type) {}
    TextureFormat format = TextureFormat::FORMAT_NONE;
    bool depthStencil = false;
    bool swapChain = false;
    bool shared = false;
    bool color = false;
};

struct DescriptorGraphNode : public ResourceNode
{
    DescriptorGraphNode(std::string name, Type type) : ResourceNode(name, type) {}
    uint32_t set = 0;
    uint32_t binding = UINT32_MAX;
    bool sampler = false;
};

struct GraphicRenderPassGraphNode : public GraphNode
{
    GraphicRenderPassGraphNode(std::string name, Type type) : GraphNode(name, type) {}
    std::string vertexShader;
    std::string framgmentShader;

    // node -> (set, binding)
    struct ResourceBindingPack
    {
        uint32_t set;
        uint32_t binding;
        Type type;
    };
    std::unordered_map<std::string, ResourceBindingPack> bindingSets;
};

struct ComputeRenderPassGraphNode : public GraphNode
{
    ComputeRenderPassGraphNode(std::string name, Type type) : GraphNode(name, type) {}
};