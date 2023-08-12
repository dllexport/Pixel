#include <gtest/gtest.h>

#include "FrameGraph/Graph.h"

TEST(DeferPasses, BasicAssertions) {

    Graph graph;
    auto depth = new GraphNode("depth");
    auto color = new GraphNode("color");
    auto normal = new GraphNode("normal");
    auto deferredPass = new GraphNode("deferredPass");
    
    deferredPass->AddOutput(depth);
    deferredPass->AddOutput(color);
    deferredPass->AddOutput(normal);

    auto clusterPass = new GraphNode("clusterPass");
    auto clusterInfo = new GraphNode("clusterInfo");

    clusterPass->AddOutput(clusterInfo);
    deferredPass->AddInput(clusterInfo);

    auto mergePass = new GraphNode("mergePass");
    mergePass->AddInput(depth);
    mergePass->AddInput(color);
    mergePass->AddInput(normal);

    graph.Add(depth);
    graph.Add(color);
    graph.Add(normal);
    graph.Add(deferredPass);
    graph.Add(clusterPass);
    graph.Add(clusterInfo);
    graph.Add(mergePass);
    
    auto result = graph.Topo();
}

TEST(CycleCheck, BasicAssertions2) {

    Graph graph;
    auto A = new GraphNode("A");
    auto B = new GraphNode("B");
    auto C = new GraphNode("C");
    auto D = new GraphNode("D");
    
    A->AddOutput(B);
    B->AddOutput(C);
    C->AddOutput(D);
    D->AddOutput(A);
 
    graph.Add(A);
    graph.Add(B);
    graph.Add(C);
    graph.Add(D);
    
    auto result = graph.Topo();
}