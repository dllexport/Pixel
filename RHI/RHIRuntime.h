#pragma once

#include <Core/IntrusivePtr.h>

#include <FrameGraph/Graph.h>

#include <RHI/RenderGroup.h>
#include <RHI/RenderGroupExecutor.h>
#include <RHI/Pipeline.h>
#include <RHI/PipelineStates.h>
#include <RHI/Buffer.h>
#include <RHI/MutableBuffer.h>
#include <RHI/Texture.h>
#include <RHI/ResourceBindingState.h>
#include <RHI/SwapChain.h>
#include <RHI/Sampler.h>
#include <RHI/AuxiliaryExecutor.h>
#include <RHI/PipelineStates.h>

class RHIRuntime : public IntrusiveCounter<RHIRuntime>
{
public:
    virtual ~RHIRuntime() = default;
    virtual IntrusivePtr<RenderGroup> CreateRenderGroup(IntrusivePtr<Graph> graph) = 0;
    virtual IntrusivePtr<Buffer> CreateBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) = 0;
    virtual IntrusivePtr<MutableBuffer> CreateMutableBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) = 0;
    virtual IntrusivePtr<Texture> CreateTexture(TextureFormat format, Texture::UsageBits type, MemoryPropertyBits memoryProperties, Texture::Extent extent, Texture::Configuration config = Texture::Configuration::Default()) = 0;
    virtual IntrusivePtr<Sampler> CreateSampler(IntrusivePtr<Texture> texture, Sampler::Configuration config = Sampler::Configuration()) = 0;
    virtual IntrusivePtr<RenderGroupExecutor> CreateRenderGroupExecutor() = 0;
    virtual IntrusivePtr<ResourceBindingState> CreateResourceBindingState(IntrusivePtr<Pipeline> pipeline) = 0;
    virtual IntrusivePtr<SwapChain> CreateSwapChain(void *handle, uint32_t width, uint32_t height, IntrusivePtr<SwapChain> oldSwapChain) = 0;
    virtual IntrusivePtr<AuxiliaryExecutor> CreateAuxiliaryExecutor() = 0;
};