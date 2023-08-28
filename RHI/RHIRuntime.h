#pragma once

#include <Core/IntrusivePtr.h>

#include <FrameGraph/Graph.h>

#include <RHI/RenderPass.h>
#include <RHI/RenderPassExecutor.h>
#include <RHI/Pipeline.h>
#include <RHI/Buffer.h>
#include <RHI/BufferArray.h>
#include <RHI/Texture.h>
#include <RHI/ResourceBindingState.h>
#include <RHI/SwapChain.h>
#include <RHI/Sampler.h>
#include <RHI/AuxiliaryExecutor.h>

class RHIRuntime : public IntrusiveCounter<RHIRuntime>
{
public:
    virtual ~RHIRuntime() = default;
    virtual IntrusivePtr<RenderPass> CreateRenderPass(IntrusivePtr<Graph> graph) = 0;
    virtual IntrusivePtr<Pipeline> CreatePipeline(IntrusivePtr<RenderPass>, std::string subPassName, PipelineStates pipelineStates) = 0;
    virtual IntrusivePtr<Buffer> CreateBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) = 0;
    virtual IntrusivePtr<BufferArray> CreateBufferArray(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) = 0;
    virtual IntrusivePtr<Texture> CreateTexture(TextureFormat format, Texture::UsageBits type, MemoryPropertyBits memoryProperties, Texture::Extent extent, Texture::Configuration config = Texture::Configuration()) = 0;
    virtual IntrusivePtr<Sampler> CreateSampler(IntrusivePtr<Texture> texture, Sampler::Configuration config = Sampler::Configuration()) = 0;
    virtual IntrusivePtr<RenderPassExecutor> CreateRenderPassExecutor() = 0;
    virtual IntrusivePtr<ResourceBindingState> CreateResourceBindingState(IntrusivePtr<Pipeline> pipeline) = 0;
    virtual IntrusivePtr<SwapChain> CreateSwapChain(void *handle, uint32_t width, uint32_t height) = 0;
    virtual IntrusivePtr<AuxiliaryExecutor> CreateAuxiliaryExecutor() = 0;
};