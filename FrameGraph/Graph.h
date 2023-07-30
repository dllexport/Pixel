#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

#include <FrameGraph/GraphNode.h>

struct Graph : IntrusiveUnsafeCounter<Graph>
{
    static IntrusivePtr<Graph> ParseRenderPassJson(std::string path);

    std::unordered_map<std::string, IntrusivePtr<GraphNode>> graphNodesMap;

    using TopoResult = std::unordered_map<uint16_t, std::vector<GraphNode *>>;
    TopoResult Topo();

    std::string name;
};