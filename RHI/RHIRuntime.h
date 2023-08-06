#pragma once

#include <Core/IntrusivePtr.h>

#include <FrameGraph/Graph.h>

#include <RHI/RenderPass.h>
#include <RHI/RenderPassExecutor.h>
#include <RHI/Pipeline.h>
#include <RHI/Buffer.h>
#include <RHI/Texture.h>
#include <RHI/ResourceBindingState.h>

class RHIRuntime : public IntrusiveCounter<RHIRuntime>
{
public:
    virtual ~RHIRuntime() = default;
    virtual IntrusivePtr<RenderPass> CreateRenderPass(IntrusivePtr<Graph> graph) = 0;
    virtual IntrusivePtr<Pipeline> CreatePipeline(IntrusivePtr<RenderPass>, std::string subPassName, PipelineStates pipelineStates) = 0;
    virtual IntrusivePtr<Buffer> CreateBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) = 0;
    virtual IntrusivePtr<Texture> CreateTexture(TextureFormat format, Texture::UsageBits type, MemoryPropertyBits memoryProperties, Texture::Extent extent, Texture::Configuration config) = 0;
    virtual IntrusivePtr<RenderPassExecutor> CreateRenderPassExecutor(IntrusivePtr<RenderPass> renderPass) = 0;
    virtual IntrusivePtr<ResourceBindingState> CreateResourceBindingState(IntrusivePtr<Pipeline>) = 0;
};