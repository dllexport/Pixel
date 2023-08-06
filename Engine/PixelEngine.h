#pragma once

#include <string>
#include <unordered_map>

#include <FrameGraph/Graph.h>

#include <RHI/PipelineStates.h>
#include <RHI/RuntimeEntry.h>

#include <Engine/DrawableBinder.h>

class RenderPass;
class Pipeline;
class Renderer;
class PixelEngine
{
public:
    PixelEngine();
    IntrusivePtr<RenderPass> RegisterRenderPass(IntrusivePtr<Graph> graph);
    IntrusivePtr<Pipeline> RegisterPipeline(std::string renderPassName, std::string subPassName, PipelineStates pipelineStates);

    IntrusivePtr<Renderer> CreateRenderer();

    IntrusivePtr<RHIRuntime> GetRHIRuntime();
    IntrusivePtr<DrawableBinder> GetDrawableBinder();

    void Frame();

    std::unordered_map<std::string, IntrusivePtr<Pipeline>> GetPipelines(IntrusivePtr<RenderPass> renderPass)
    {
        return pipelineTemplates[renderPass];
    }

private:
    std::unordered_map<std::string, IntrusivePtr<RenderPass>> renderPassTemplates;
    std::unordered_map<IntrusivePtr<RenderPass>, std::unordered_map<std::string, IntrusivePtr<Pipeline>>> pipelineTemplates;
    IntrusivePtr<RHIRuntime> rhiRuntime;

    friend class Renderer;
    std::vector<IntrusivePtr<Renderer>> renderers;

    // manage uniform resource binding
    IntrusivePtr<DrawableBinder> drawableBinder;
};