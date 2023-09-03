#pragma once

#include <vector>
#include <map>
#include <queue>
#include <optional>

#include <FrameGraph/GraphNode.h>

struct Graph : IntrusiveUnsafeCounter<Graph>
{
    static IntrusivePtr<Graph> ParseRenderPassJson(std::string path);

    std::unordered_map<std::string, IntrusivePtr<GraphNode>> graphNodesMap;
    std::vector<IntrusivePtr<GraphNode>> resourceNodes;

    struct TopoResult
    {
        std::map<uint16_t, std::vector<GraphNode *>> levels;
        std::map<uint16_t, std::vector<GraphNode *>> levelsRenderPassOnly;
        uint16_t maxLevel;
        uint16_t maxLevelRenderPassOnly;

        uint16_t LevelOf(GraphNode * node) {
            
        }
    };
    TopoResult &Topo();

    std::string name;

private:
    std::optional<TopoResult> topoResultCache;
};