#pragma once

#include <FrameGraph/Graph.h>

#include <RHI/Memory.h>
#include <RHI/RHIRuntime.h>
#include <RHI/VulkanRuntime/Context.h>

#include <RHI/VulkanRuntime/RenderPassExecutor.h>

class VulkanRuntime : public RHIRuntime
{
public:
    VulkanRuntime();
    virtual ~VulkanRuntime() override;

    IntrusivePtr<Context> GetContext();

    virtual IntrusivePtr<RenderPass> CreateRenderPass(IntrusivePtr<Graph> graph) override;
    virtual IntrusivePtr<Pipeline> CreatePipeline(IntrusivePtr<RenderPass> renderPass, std::string subPassName, PipelineStates pipelineStates) override;
    virtual IntrusivePtr<Buffer> CreateBuffer(Buffer::TypeBits type, MemoryPropertyBits, uint32_t size) override;
    virtual IntrusivePtr<Texture> CreateTexture(TextureFormat format, Texture::UsageBits type, MemoryPropertyBits memoryProperties, Texture::Extent extent, Texture::Configuration config) override;
    virtual IntrusivePtr<RenderPassExecutor> CreateRenderPassExecutor() override;
    virtual IntrusivePtr<ResourceBindingState> CreateResourceBindingState(IntrusivePtr<Pipeline> pipeline) override;
    virtual IntrusivePtr<SwapChain> CreateSwapChain(void *handle, uint32_t width, uint32_t height) override;

private:
    IntrusivePtr<Context> context = nullptr;
};