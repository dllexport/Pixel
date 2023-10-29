#pragma once

#include <FrameGraph/Graph.h>

#include <RHI/Memory.h>
#include <RHI/RHIRuntime.h>
#include <RHI/VulkanRuntime/Context.h>

class VulkanRuntime : public RHIRuntime
{
public:
    VulkanRuntime();
    virtual ~VulkanRuntime() override;

    IntrusivePtr<Context> GetContext();

    virtual IntrusivePtr<RenderGroup> CreateRenderGroup(IntrusivePtr<Graph> graph) override;
    virtual IntrusivePtr<Pipeline> CreatePipeline(IntrusivePtr<RenderGroup> renderPass, std::string subPassName, PipelineStates pipelineStates) override;
    virtual IntrusivePtr<Pipeline> CreateComputePipeline(IntrusivePtr<RenderGroup>, std::string subPassName) override;
    virtual IntrusivePtr<Buffer> CreateBuffer(Buffer::TypeBits type, MemoryPropertyBits, uint32_t size) override;
    virtual IntrusivePtr<MutableBuffer> CreateMutableBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) override;
    virtual IntrusivePtr<Texture> CreateTexture(TextureFormat format, Texture::UsageBits type, MemoryPropertyBits memoryProperties, Texture::Extent extent, Texture::Configuration config) override;
    virtual IntrusivePtr<Sampler> CreateSampler(IntrusivePtr<Texture> texture, Sampler::Configuration config) override;
    virtual IntrusivePtr<RenderGroupExecutor> CreateRenderGroupExecutor() override;
    virtual IntrusivePtr<ResourceBindingState> CreateResourceBindingState(IntrusivePtr<Pipeline> pipeline) override;
    virtual IntrusivePtr<SwapChain> CreateSwapChain(void *handle, uint32_t width, uint32_t height, IntrusivePtr<SwapChain> oldSwapChain) override;
    virtual IntrusivePtr<AuxiliaryExecutor> CreateAuxiliaryExecutor() override;

private:
    IntrusivePtr<Context> context = nullptr;
};