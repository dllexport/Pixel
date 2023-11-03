#include <RHI/RenderGroup.h>

RenderGroup::RenderGroup(IntrusivePtr<Graph> graph) : graph(graph)
{
}

std::string RenderGroup::Name()
{
    return graph->GetName();
}

const IntrusivePtr<Graph> RenderGroup::GetGraph()
{
    return graph;
}

std::vector<std::string> ExportResources()
{
    return {};
}
