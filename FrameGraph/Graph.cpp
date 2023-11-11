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

IntrusivePtr<Graph> Graph::ParseRenderPassJsonRawString(std::string jsonStr)
{
    IntrusivePtr<Graph> graph = new Graph;

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
            auto crp = new ComputeRenderPassGraphNode(subpass.name, GraphNode::COMPUTE_PASS);
            crp->computeShader = subpass.shaders.compute;
            node = crp;
        }

        if (!subpass.dependencies.empty())
            node->As<RenderPassGraphNode *>()->dependencies.insert(subpass.dependencies.cbegin(), subpass.dependencies.cend());

        node->passName = subpass.name;
        node->groupName = json.name;
        resourceNodes.push_back(node);
        resolvedMap[node->GlobalName()] = node;

        for (auto input : subpass.inputs)
        {
            ResourceNode *inputNode;
            if (input.type == "attachment")
            {
                auto attachment = new AttachmentGraphNode(input.name, GraphNode::ATTACHMENT);
                attachment->depthStencil = input.depthStencil;
                // swapchain attachment can not be input node
                assert(!input.swapChain);
                attachment->shared = input.shared;
                attachment->clear = input.clear;
                attachment->input = true;
                attachment->format = TranslateFormat(input.format);
                attachment->color = attachment->depthStencil ? false : true;
                inputNode = attachment;
            }
            else if (input.type == "buffer" || input.type == "sampler" || input.type == "ssbo")
            {
                auto dn = new DescriptorGraphNode(input.name, input.type == "sampler" ? GraphNode::SAMPLER : GraphNode::BUFFER);
                dn->set = 0;
                inputNode = dn;
            }

            // all input nodes are ResourceNode
            inputNode->binding = input.binding;

            inputNode->passName = node->name;
            inputNode->groupName = json.name;

            // save which subpass use this node as input
            inputNode->inputSubPassNames.insert(subpass.name);

            if (!resolvedMap.count(inputNode->GlobalName()))
            {
                if (input.shared || input.swapChain)
                {
                    graph->sharedResourceKeys.insert(input.name);
                }
                resolvedMap[inputNode->GlobalName()] = inputNode;
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
                auto attachment = new AttachmentGraphNode(output.name, GraphNode::ATTACHMENT);
                attachment->depthStencil = output.depthStencil;
                attachment->swapChain = output.swapChain;
                attachment->shared = output.shared || output.swapChain;
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
            outputNode->groupName = json.name;

            if (!resolvedMap.count(outputNode->GlobalName()))
            {
                if (output.shared || output.swapChain)
                {
                    graph->sharedResourceKeys.insert(output.name);
                }
                resolvedMap[outputNode->GlobalName()] = outputNode;
            }
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
        // inputs must be descriptors
        for (unsigned i = 0; i < passNode->inputs.size(); i++)
        {
            auto resNode = (DescriptorGraphNode *)passNode->inputs[i].get();
            passNode->bindingSets[resNode->GlobalName()] = {resNode->set, resNode->binding, passNode->inputs[i]->type};
        }
        // TODO, handle outputs (BUFFER SSBO)
    }

    // append subpass dependency
    //    for (auto &subpass : json.subpasses)
    //    {
    //        auto node = resolvedMap[subpass.name];
    //        for (auto &dependency : subpass.subpass_dependency)
    //        {
    //            if (resolvedMap.count(dependency))
    //            {
    //                auto dep = resolvedMap[dependency];
    //                node->inputs.push_back(dep);
    //                dep->outputs.push_back(node);
    //            }
    //            // if dependency not exist, it's resolved in global scope
    //        }
    //    }

    graph->name = json.name;
    graph->graphNodesMap = resolvedMap;
    return graph;
}

IntrusivePtr<Graph> Graph::ParseRenderPassJson(std::string path)
{
    auto jsonStr = ReadStringFile(path);

    return ParseRenderPassJsonRawString(jsonStr);
}

template <typename Iter, typename Q>
void push_range(Q &q, Iter begin, Iter end)
{
    for (; begin != end; ++begin)
        q.push(*begin);
}

std::string Graph::GetNodeLocalName(const std::string &str)
{
    std::vector<std::string> splitted;
    boost::algorithm::split_regex(splitted, str, boost::regex("::"));
    return splitted.back();
};

void Graph::DumpNodeMapString()
{
    for (auto &[level, nodes] : graphNodesMap)
    {
        spdlog::info("{} {}", level, nodes->name);
    }
}

Graph::TopoResult &Graph::Topo()
{
    if (topoResultCache.has_value())
    {
        return topoResultCache.value();
    }

    std::map<uint16_t, std::vector<GraphNode *>> result;

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
        indegreeMap[v->ScopeName()] += v->inputs.size();
    }

    for (auto &[k, v] : indegreeMap)
    {
        spdlog::info("indegreeMap {} {}", k, v);
    }

    std::queue<GraphNode *> topoQueue;
    for (auto &[k, v] : graphNodesMap)
    {
        if (indegreeMap[v->ScopeName()] == 0)
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
                indegreeMap[v->ScopeName()]--;

                spdlog::info("{} down to {}", v->GlobalName(), indegreeMap[v->ScopeName()]);
                if (indegreeMap[v->ScopeName()] == 0)
                {
                    for (auto &[k, n] : graphNodesMap)
                    {
                        spdlog::info("comp {} {}", v->name, n->name);
                        if (v->name == n->name)
                        {
                            spdlog::info("adding {}", n->GlobalName());
                            tempTopoQueue.push(n.get());
                        }
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
            spdlog::info("{} {}", level, node->GlobalName());
        }
    }

    // key -> passes
    std::unordered_map<std::string, std::vector<std::string>> outputNameSet;
    for (auto &[level, nodes] : result)
    {
        for (auto &node : nodes)
        {
            for (auto &output : node->outputs)
            {
                if (output->type == GraphNode::GRAPHIC_PASS || output->type == GraphNode::COMPUTE_PASS)
                {
                    continue;
                }
                if (outputNameSet.count(output->GlobalName()))
                {
                    spdlog::warn("level {} concurrent write to {}", level, output->GlobalName());
                }
                outputNameSet[output->GlobalName()].push_back(node->GlobalName());
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

IntrusivePtr<Graph> Graph::Merge(std::vector<IntrusivePtr<Graph>> graphs)
{
    auto graph = new Graph();

    for (auto &g : graphs)
    {
        // merge graphNodesMap
        for (auto &[k, v] : g->graphNodesMap)
        {

            // check if key exist(is shared)
            if (graph->graphNodesMap.count(k) == 0)
                graph->graphNodesMap[k] = v;
            else
            {
                auto &dstInputs = graph->graphNodesMap[k]->inputs;
                auto &srcInputs = g->graphNodesMap[k]->inputs;
                dstInputs.insert(dstInputs.end(), srcInputs.begin(), srcInputs.end());

                auto &dstOutputs = graph->graphNodesMap[k]->outputs;
                auto &srcOutputs = g->graphNodesMap[k]->outputs;
                dstOutputs.insert(dstOutputs.end(), srcOutputs.begin(), srcOutputs.end());
            }
        }

        for (auto s : g->sharedResourceKeys)
            graph->sharedResourceKeys.insert(s);
    }

    for (auto sharedKey : graph->sharedResourceKeys)
    {
        auto inputs = graph->graphNodesMap[sharedKey]->inputs;
        for (auto &input : inputs)
        {
            auto rgn = input->As<RenderPassGraphNode *>();
            spdlog::info("checking dependencies of {}", rgn->GlobalName());
            for (auto &dep : rgn->dependencies)
            {
                if (!graph->graphNodesMap.count(dep))
                {
                    spdlog::warn("{} has dependency of {} in json, but not found", input->GlobalName(), dep);
                    continue;
                }
                auto from = graph->graphNodesMap.at(dep);
                auto to = input;
                spdlog::info("linking {} to {}", from->GlobalName(), to->GlobalName());

                from->outputs.push_back(to);
                to->inputs.push_back(from);
            }
        }
    }

    return graph;
}
