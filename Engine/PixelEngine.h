#pragma once

#include <string>
#include <unordered_map>

#include <FrameGraph/Graph.h>

#include <RHI/PipelineStates.h>
#include <RHI/RuntimeEntry.h>
#include <RHI/AuxiliaryExecutor.h>

class RenderGroup;
class Pipeline;
class Renderer;
class PixelEngine : public IntrusiveCounter<PixelEngine>
{
public:
    PixelEngine();
    ~PixelEngine();
    IntrusivePtr<RenderGroup> RegisterRenderGroup(IntrusivePtr<Graph> graph);

    IntrusivePtr<Renderer> CreateRenderer();

    IntrusivePtr<RHIRuntime> &GetRHIRuntime();

    void Frame();

    IntrusivePtr<AuxiliaryExecutor> GetAuxiliaryExecutor()
    {
        return auxExecutor;
    }

private:
    std::unordered_map<std::string, IntrusivePtr<RenderGroup>> renderGroupTemplates;
    IntrusivePtr<RHIRuntime> rhiRuntime;

    friend class Renderer;
    std::vector<IntrusivePtr<Renderer>> renderers;

    IntrusivePtr<AuxiliaryExecutor> auxExecutor;
};