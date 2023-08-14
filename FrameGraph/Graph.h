#pragma once

#include <vector>
#include <unordered_map>
#include <queue>
#include <optional>

#include <FrameGraph/GraphNode.h>

struct Graph : IntrusiveUnsafeCounter<Graph>
{
    static IntrusivePtr<Graph> ParseRenderPassJson(std::string path);

    std::unordered_map<std::string, IntrusivePtr<GraphNode>> graphNodesMap;

    struct TopoResult
    {
        std::unordered_map<uint16_t, std::vector<GraphNode *>> levels;
        std::unordered_map<uint16_t, std::vector<GraphNode *>> levelsRenderPassOnly;
    };
    TopoResult &Topo();

    std::string name;

private:
    std::optional<TopoResult> topoResultCache;
};