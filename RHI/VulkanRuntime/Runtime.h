#pragma once

#include <FrameGraph/Graph.h>

#include <RHI/RHIRuntime.h>
#include <RHI/VulkanRuntime/Context.h>

class VulkanRuntime : public RHIRuntime
{
public:
    VulkanRuntime();

    IntrusivePtr<Context> GetContext();

    virtual IntrusivePtr<RenderPass> CreateRenderPass(IntrusivePtr<Graph> graph) override;
    virtual IntrusivePtr<Pipeline> CreatePipeline(IntrusivePtr<RenderPass> renderPass, std::string subPassName, PipelineStates pipelineStates) override;

private:
    IntrusivePtr<Context> context = nullptr;
};