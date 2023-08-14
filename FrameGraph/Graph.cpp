#include "Graph.h"

#include <string>
#include <unordered_set>

#include <Core/ReadFile.h>

#include <FrameGraph/GraphNodeJson.h>
#include <FrameGraph/GraphNode.h>

static TextureFormat TranslateFormat(std::string formatStr)
{
#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

#define DECL_FORMAT_CASE(FORMAT)        \
    if (formatStr == STRINGIFY(FORMAT)) \
    {                                   \
        return TextureFormat::FORMAT;   \
    }

    DECL_FORMAT_CASE(FORMAT_B8G8R8A8_SRGB)
    DECL_FORMAT_CASE(FORMAT_B8G8R8A8_UNORM)
    DECL_FORMAT_CASE(FORMAT_R16G16B16A16_SFLOAT)
    DECL_FORMAT_CASE(FORMAT_D16_UNORM)

    return TextureFormat::FORMAT_NONE;

#undef DECL_FORMAT_CASE
#undef STRINGIFY2
#undef STRINGIFY
}

IntrusivePtr<Graph> Graph::ParseRenderPassJson(std::string path)
{
    auto jsonStr = ReadStringFile(path);
    JS::ParseContext context(jsonStr);
    RenderPassJson json;
    auto error = context.parseTo(json);

    std::unordered_map<std::string, IntrusivePtr<GraphNode>> resolvedMap;
    std::vector<IntrusivePtr<GraphNode>> resourceNodes;

    for (auto subpass : json.subpasses)
    {
        IntrusivePtr<GraphNode> node;
        if (subpass.type == "graphic")
        {
            auto grp = new GraphicRenderPassGraphNode(subpass.name, GraphNode::GRAPHIC_PASS);
            grp->vertexShader = subpass.shaders.vertex;
            grp->framgmentShader = subpass.shaders.fragment;
            node = grp;
        }
        else if (subpass.type == "compute")
        {
            node = new ComputeRenderPassGraphNode(subpass.name, GraphNode::COMPUTE_PASS);
        }

        resourceNodes.push_back(node);
        resolvedMap[node->name] = node;

        for (auto input : subpass.inputs)
        {
            GraphNode *inputNode;
            if (input.type == "attachment")
            {
                auto attachent = new AttachmentGraphNode(input.name, GraphNode::ATTACHMENT);
                attachent->depthStencil = input.depthStencil;
                attachent->swapChain = input.swapChain;
                attachent->shared = input.shared;
                attachent->format = TranslateFormat(input.format);
                inputNode = attachent;
            }
            else if (input.type == "buffer")
            {
                inputNode = new DescriptorGraphNode(input.name, GraphNode::BUFFER);
            }
            else if (input.type == "reference")
            {
                inputNode = new GraphNode(input.name, GraphNode::REFERENCE);
            }

            if (input.type != "reference")
            {
                resolvedMap[input.name] = inputNode;
            }

            resourceNodes.push_back(inputNode);
            node->inputs.push_back(inputNode);
            inputNode->outputs.push_back(node);
        }

        for (auto output : subpass.outputs)
        {
            GraphNode *outputNode;
            if (output.type == "attachment")
            {
                auto attachent = new AttachmentGraphNode(output.name, GraphNode::ATTACHMENT);
                attachent->depthStencil = output.depthStencil;
                attachent->swapChain = output.swapChain;
                attachent->shared = output.shared;
                attachent->format = TranslateFormat(output.format);
                outputNode = attachent;
            }
            else if (output.type == "buffer")
            {
                outputNode = new DescriptorGraphNode(output.name, GraphNode::BUFFER);
            }
            else if (output.type == "reference")
            {
                outputNode = new GraphNode(output.name, GraphNode::REFERENCE);
            }

            if (output.type != "reference")
            {
                resolvedMap[output.name] = outputNode;
            }

            resourceNodes.push_back(outputNode);
            node->outputs.push_back(outputNode);
            outputNode->inputs.push_back(node);
        }
    }

    auto mergeGraphNode = [](GraphNode *source, GraphNode *target)
    {
        {
            std::unordered_set<IntrusivePtr<GraphNode>> set;
            for (auto n : source->inputs)
            {
                set.insert(n);
            }
            for (auto n : target->inputs)
            {
                set.insert(n);
            }

            std::vector<IntrusivePtr<GraphNode>> inputs(set.size());
            std::copy(set.begin(), set.end(), inputs.begin());
            target->inputs.swap(inputs);
        }

        {
            std::unordered_set<IntrusivePtr<GraphNode>> set;
            for (auto n : source->outputs)
            {
                set.insert(n);
            }
            for (auto n : target->outputs)
            {
                set.insert(n);
            }

            std::vector<IntrusivePtr<GraphNode>> outputs(set.size());
            std::copy(set.begin(), set.end(), outputs.begin());
            target->outputs.swap(outputs);
        }
    };

    // handle reference
    for (auto &node : resourceNodes)
    {
        for (auto &n : node->inputs)
        {
            if (n->type == GraphNode::REFERENCE)
            {
                auto target = resolvedMap[n->name];
                mergeGraphNode(n.get(), target.get());
                n = target;
            }
        }

        for (auto &n : node->outputs)
        {
            if (n->type == GraphNode::REFERENCE)
            {
                auto target = resolvedMap[n->name];
                mergeGraphNode(n.get(), target.get());
                n = target;
            }
        }
    }

    // remove reference
    for (auto &[k, v] : resolvedMap)
    {
        if (v->type == GraphNode::REFERENCE)
        {
            resolvedMap.erase(k);
        }
    }

    // append subpass dependency
    for (auto subpass : json.subpasses)
    {
        auto node = resolvedMap[subpass.name];
        for (auto &dependency : subpass.subpass_dependency)
        {
            auto dep = resolvedMap[dependency];
            node->inputs.push_back(dep);
            dep->outputs.push_back(node);
        }
    }

    auto graph = new Graph;
    graph->name = json.name;
    graph->graphNodesMap = resolvedMap;
    return graph;
}

Graph::TopoResult& Graph::Topo()
{
    if (topoResultCache.has_value())
    {
        return topoResultCache.value();
    }

    std::unordered_map<uint16_t, std::vector<GraphNode *>> result;

    std::queue<GraphNode *> topoQueue;
    for (auto &[k, v] : graphNodesMap)
    {
        v->inputDegree = v->inputs.size();
        if (v->inputs.empty())
        {
            topoQueue.push(v.get());
        }
    }

    uint16_t level = 0;
    while (!topoQueue.empty())
    {
        std::queue<GraphNode *> tempTopoQueue;

        while (!topoQueue.empty())
        {
            auto front = topoQueue.front();
            topoQueue.pop();

            result[level].push_back(front);

            for (auto &v : front->outputs)
            {
                v->inputDegree--;
                if (v->inputDegree == 0)
                {
                    tempTopoQueue.push(v.get());
                }
            }
        }

        level++;
        topoQueue.swap(tempTopoQueue);
    }

    for (auto &[level, nodes] : result)
    {
        for (auto &node : nodes)
        {
            printf("%s\n", node->name.c_str());
        }
        printf("-------\n");
    }

    for (auto &[level, nodes] : result)
    {
        std::unordered_set<IntrusivePtr<GraphNode>> outputSet;
        for (auto &node : nodes)
        {
            for (auto &output : node->outputs)
            {
                if (output->type == GraphNode::GRAPHIC_PASS || output->type == GraphNode::COMPUTE_PASS)
                {
                    continue;
                }
                if (outputSet.count(output))
                {
                    printf("level %d concurrent write to %s\n", level, output->name.c_str());
                }
                else
                {
                    outputSet.insert(output);
                }
            }
        }
    }

    std::unordered_map<uint16_t, std::vector<GraphNode *>> passOnly;
    for (auto &[k, v] : result)
    {
        for (auto &n : v)
        {
            if (n->type == GraphNode::Type::GRAPHIC_PASS || n->type == GraphNode::Type::COMPUTE_PASS)
            {
                passOnly[k].push_back(n);
            }
        }
    }

    topoResultCache = TopoResult{
        .levels = result,
        .levelsRenderPassOnly = passOnly};

    return topoResultCache.value();
}