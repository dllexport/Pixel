#include <RHI/RenderPass.h>

std::string RenderPass::Name()
{
    return this->graph->name;
}

RenderPass::RenderPass(IntrusivePtr<Graph> graph) : graph(graph)
{
}
