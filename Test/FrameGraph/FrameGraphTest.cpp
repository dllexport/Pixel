#include <gtest/gtest.h>

#include <FrameGraph/Graph.h>

TEST(FrameGraphTest, SingleGraphicPass)
{
    auto graph = Graph::ParseRenderPassJson("singleRenderPass.json");
    EXPECT_EQ(graph->name, "RP 0");
    EXPECT_EQ(graph->graphNodesMap.count("pass 0"), 1);

    auto passNode = (GraphicRenderPassGraphNode *)(graph->graphNodesMap["pass 0"].get());
    EXPECT_EQ(passNode->name, "pass 0");
    EXPECT_EQ(passNode->type, GraphNode::GRAPHIC_PASS);
    EXPECT_EQ(passNode->inputs.size(), 4);
    EXPECT_EQ(passNode->outputs.size(), 3);
    EXPECT_EQ(passNode->vertexShader, "vert.spv");
    EXPECT_EQ(passNode->framgmentShader, "frag.spv");

    EXPECT_EQ(graph->graphNodesMap.count("pass 0::ubo 0"), 1);
    EXPECT_EQ(graph->graphNodesMap.count("pass 0::ubo 1"), 1);
    EXPECT_EQ(graph->graphNodesMap.count("pass 0::sampler 0"), 1);
    EXPECT_EQ(graph->graphNodesMap.count("pass 0::sampler 1"), 1);
    EXPECT_EQ(graph->graphNodesMap.count("pass 0::attachment 0"), 1);
    EXPECT_EQ(graph->graphNodesMap.count("pass 0::attachment 1"), 1);
    EXPECT_EQ(graph->graphNodesMap.count("pass 0::depth"), 1);

    auto topo = graph->Topo();
    EXPECT_EQ(topo.levels.size(), 3);

    std::unordered_set<std::string> level1 = {"pass 0::ubo 0", "pass 0::ubo 1", "pass 0::sampler 0", "pass 0::sampler 1"};
    for (auto node : topo.levels[0])
        level1.erase(node->GlobalName());
    EXPECT_EQ(level1.size(), 0);

    std::unordered_set<std::string> level2 = {"pass 0"};
    for (auto node : topo.levels[1])
        level2.erase(node->name);
    EXPECT_EQ(level2.size(), 0);

    std::unordered_set<std::string> level3 = {"pass 0::attachment 0", "pass 0::attachment 1", "pass 0::depth"};
    for (auto node : topo.levels[2])
        level3.erase(node->GlobalName());
    EXPECT_EQ(level3.size(), 0);
}

TEST(FrameGraphTest, MultipleNodeReference)
{
    auto graph = Graph::ParseRenderPassJson("mutipleRenderPass.json");
    graph->Topo();
}
