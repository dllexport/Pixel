#pragma once

#include <string>
#include <vector>

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

    // get all outputs of outputs, filtered by type
    std::vector<IntrusivePtr<GraphNode>> TraceAllOutputs(Type type, uint32_t traceLevel)
    {
        if (traceLevel == 0)
        {
            return {};
        }

        std::vector<IntrusivePtr<GraphNode>> result;
        for (auto &output : outputs)
        {
            if (output->type == type)
            {
                result.push_back(output);
            }

            for (auto output2 : output->outputs)
            {
                auto nestedResult = output2->TraceAllOutputs(type, --traceLevel);
                result.insert(result.end(), nestedResult.begin(), nestedResult.end());
            }
        }
        return result;
    }

    friend class Graph;
    std::vector<IntrusivePtr<GraphNode>> inputs;
    std::vector<IntrusivePtr<GraphNode>> outputs;
};

struct AttachmentGraphNode : public GraphNode
{
    AttachmentGraphNode(std::string name, Type type) : GraphNode(name, type) {}
    TextureFormat format = TextureFormat::FORMAT_NONE;
    bool depthStencil = false;
    bool swapChain = false;
    bool shared = false;
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
    std::string vertexShader;
    std::string framgmentShader;
};

struct ComputeRenderPassGraphNode : public GraphNode
{
    ComputeRenderPassGraphNode(std::string name, Type type) : GraphNode(name, type) {}
};