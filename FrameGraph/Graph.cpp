#include "Graph.h"

#include <string>
#include <unordered_set>

#include <spdlog/spdlog.h>

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

// replace reference node with instance
static void ResolveReferenceNode(std::unordered_map<std::string, IntrusivePtr<GraphNode>> &resolvedMap, std::vector<IntrusivePtr<GraphNode>> &resourceNodes)
{
    // merge ref source to target, including all inputs && outputs
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

        for (auto subPassName : source->inputSubPassNames)
        {
            target->inputSubPassNames.insert(subPassName);
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
}

IntrusivePtr<Graph> Graph::ParseRenderPassJson(std::string path)
{
    auto jsonStr = ReadStringFile(path);
    JS::ParseContext context(jsonStr);
    RenderPassJson json;
    auto error = context.parseTo(json);

    // none reference node save here
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
                auto attachment = new AttachmentGraphNode(input.name, GraphNode::ATTACHMENT);
                attachment->depthStencil = input.depthStencil;
                attachment->swapChain = input.swapChain;
                attachment->shared = input.shared;
                attachment->clear = input.clear;
                attachment->format = TranslateFormat(input.format);
                inputNode = attachment;
            }
            else if (input.type == "buffer" || input.type == "sampler")
            {
                inputNode = new DescriptorGraphNode(input.name, input.type == "sampler" ? GraphNode::SAMPLER : GraphNode::BUFFER);
            }
            else if (input.type == "reference")
            {
                inputNode = new GraphNode(input.name, GraphNode::REFERENCE);
            }

            if (input.type != "reference")
            {
                resolvedMap[input.name] = inputNode;
            }

            // save which subpass use this node as input
            inputNode->inputSubPassNames.insert(subpass.name);

            resourceNodes.push_back(inputNode);
            node->inputs.push_back(inputNode);
            inputNode->outputs.push_back(node);
        }

        for (auto output : subpass.outputs)
        {
            GraphNode *outputNode;
            if (output.type == "attachment")
            {
                auto attachment = new AttachmentGraphNode(output.name, GraphNode::ATTACHMENT);
                attachment->depthStencil = output.depthStencil;
                attachment->swapChain = output.swapChain;
                attachment->shared = output.shared;
                attachment->clear = output.clear;
                attachment->color = attachment->depthStencil ? false : true;
                attachment->format = TranslateFormat(output.format);
                outputNode = attachment;
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

    ResolveReferenceNode(resolvedMap, resourceNodes);

    for (auto [name, node] : resolvedMap)
    {
        if (node->type != GraphNode::GRAPHIC_PASS)
        {
            continue;
        }
        auto passNode = (GraphicRenderPassGraphNode *)node.get();
        for (int i = 0; i < passNode->inputs.size(); i++)
        {
            auto resNode = (ResourceNode *)passNode->inputs[i].get();
            passNode->bindingSets[resNode->name] = {resNode->set, resNode->binding == UINT32_MAX ? i : resNode->binding, resNode->type};
        }
        // TODO, handle outputs (BUFFER SSBO)
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

Graph::TopoResult &Graph::Topo()
{
    if (topoResultCache.has_value())
    {
        return topoResultCache.value();
    }

    std::map<uint16_t, std::vector<GraphNode *>> result;

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
            spdlog::info("{}", node->name);
        }
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
                    spdlog::info("level {} concurrent write to {}", level, output->name);
                }
                else
                {
                    outputSet.insert(output);
                }
            }
        }
    }

    std::map<uint16_t, std::vector<GraphNode *>> passOnly;
    uint16_t levelPassOnly = 0;
    for (auto &[k, v] : result)
    {
        bool insertAny = false;
        for (auto &n : v)
        {
            if (n->type == GraphNode::Type::GRAPHIC_PASS || n->type == GraphNode::Type::COMPUTE_PASS)
            {
                passOnly[levelPassOnly].push_back(n);
                insertAny = true;
            }
        }
        if (insertAny)
            levelPassOnly++;
    }

    topoResultCache = TopoResult{
        .levels = result,
        .levelsRenderPassOnly = passOnly,
        .maxLevel = uint16_t(level - 1),
        .maxLevelRenderPassOnly = uint16_t(levelPassOnly - 1)};

    return topoResultCache.value();
}