#include "Graph.h"

#include <string>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <unordered_set>

#include <spdlog/spdlog.h>

#include <Core/ReadFile.h>

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

bool isPipelineNode(IntrusivePtr<GraphNode> node)
{
    return node->type == GraphNode::Type::GRAPHIC_PASS || node->type == GraphNode::Type::COMPUTE_PASS;
}

IntrusivePtr<Graph> Graph::ParseRenderPassJson(std::string path)
{
    IntrusivePtr<Graph> graph = new Graph;

    auto jsonStr = ReadStringFile(path);
    JS::ParseContext context(jsonStr);
    auto &json = graph->json;
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
                if (attachment->shared)
                    graph->sharedAttachments.insert(attachment->name);
                attachment->clear = input.clear;
                attachment->format = TranslateFormat(input.format);
                inputNode = attachment;
            }
            else if (input.type == "buffer" || input.type == "sampler" || input.type == "ssbo")
            {
                inputNode = new DescriptorGraphNode(input.name, input.type == "sampler" ? GraphNode::SAMPLER : GraphNode::BUFFER);
            }

            inputNode->passName = node->name;
            // save which subpass use this node as input
            inputNode->inputSubPassNames.insert(subpass.name);

            if (!resolvedMap.count(inputNode->name))
                resolvedMap[node->name + "::" + inputNode->name] = inputNode;
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
                if (attachment->shared)
                    graph->sharedAttachments.insert(attachment->name);
                attachment->clear = output.clear;
                attachment->color = attachment->depthStencil ? false : true;
                attachment->format = TranslateFormat(output.format);
                outputNode = attachment;
            }
            else if (output.type == "buffer" || output.type == "ssbo")
            {
                outputNode = new DescriptorGraphNode(output.name, GraphNode::BUFFER);
            }

            outputNode->passName = node->name;

            if (!resolvedMap.count(outputNode->name))
                resolvedMap[node->name + "::" + outputNode->name] = outputNode;
            resourceNodes.push_back(outputNode);
            node->outputs.push_back(outputNode);
            outputNode->inputs.push_back(node);
        }
    }

    for (auto [name, node] : resolvedMap)
    {
        if (!isPipelineNode(node))
            continue;

        auto passNode = (RenderPassGraphNode *)node.get();
        for (unsigned i = 0; i < passNode->inputs.size(); i++)
        {
            auto resNode = (ResourceNode *)passNode->inputs[i].get();
            passNode->bindingSets[resNode->name] = {0, i, passNode->inputs[i]->type};
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

    graph->name = json.name;
    graph->graphNodesMap = resolvedMap;
    return graph;
}

template <typename Iter, typename Q>
void push_range(Q &q, Iter begin, Iter end)
{
    for (; begin != end; ++begin)
        q.push(*begin);
}

Graph::TopoResult &Graph::Topo()
{
    if (topoResultCache.has_value())
    {
        return topoResultCache.value();
    }

    std::map<uint16_t, std::vector<GraphNode *>> result;

    auto getLocalName = [](const std::string &str)
    {
        std::vector<std::string> splitted;
        boost::algorithm::split_regex(splitted, str, boost::regex("::"));
        return splitted.back();
    };

    auto getSameNodes = [&](const std::string &name)
    {
        std::vector<GraphNode *> nodes;
        for (auto &[k, v] : this->graphNodesMap)
        {
            if (v->name == name)
            {
                nodes.push_back(v.get());
            }
        }
        return nodes;
    };

    std::unordered_map<std::string, uint32_t> indegreeMap;
    for (auto &[k, v] : graphNodesMap)
    {
        indegreeMap[v->name] += v->inputs.size();
    }

    for (auto &[level, nodes] : graphNodesMap)
    {
        spdlog::info("{} {}", level, nodes->name);
    }

    std::queue<GraphNode *> topoQueue;
    for (auto &[k, v] : graphNodesMap)
    {
        auto localName = getLocalName(k);
        if (indegreeMap[localName] == 0)
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
                indegreeMap[v->name]--;
                if (indegreeMap[v->name] == 0)
                {
                    for (auto &[k, n] : graphNodesMap)
                    {
                        if (v->name == n->name)
                            tempTopoQueue.push(n.get());
                    }
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
            spdlog::info("{} {}", level, node->passName + "::" + node->name);
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