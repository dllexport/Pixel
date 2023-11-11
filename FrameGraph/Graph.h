#pragma once

#include <vector>
#include <unordered_set>
#include <map>
#include <queue>
#include <optional>

#include <FrameGraph/GraphNode.h>
#include <FrameGraph/GraphNodeJson.h>

class Graph : public IntrusiveUnsafeCounter<Graph>
{
public:
    Graph() = default;

    struct TopoResult
    {
        std::map<uint16_t, std::vector<GraphNode *>> levels;
        std::map<uint16_t, std::vector<GraphNode *>> levelsRenderPassOnly;
        uint16_t maxLevel;
        uint16_t maxLevelRenderPassOnly;
    };

    static IntrusivePtr<Graph> ParseRenderPassJson(std::string path);
    static IntrusivePtr<Graph> ParseRenderPassJsonRawString(std::string jsonStr);
    TopoResult &Topo();

    RenderPassJson &GetJson()
    {
        return json;
    }

    IntrusivePtr<GraphNode> GetNode(std::string passName, std::string nodeName)
    {
        return this->graphNodesMap[passName + "::" + nodeName];
    }

    static std::string GetNodeLocalName(const std::string &str);

    const std::string &GetName()
    {
        return name;
    }

    const std::unordered_map<std::string, IntrusivePtr<GraphNode>> &GetNodeMap()
    {
        return this->graphNodesMap;
    }

    const std::unordered_set<std::string> &GetSharedResourceKeys()
    {
        return sharedResourceKeys;
    }

    void DumpNodeMapString();

    static IntrusivePtr<Graph> Merge(std::vector<IntrusivePtr<Graph>> graphs);

private:
    // full scope name -> graph node
    std::unordered_map<std::string, IntrusivePtr<GraphNode>> graphNodesMap;

    std::unordered_set<std::string> sharedResourceKeys;

    std::string name;

    std::optional<TopoResult> topoResultCache;
    RenderPassJson json;
};
