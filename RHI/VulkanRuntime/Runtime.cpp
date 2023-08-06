#include <RHI/VulkanRuntime/Runtime.h>

#include <spdlog/spdlog.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/ContextBuilder.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/RenderPassExecutor.h>
#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/Buffer.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/ResourceBindingState.h>

VulkanRuntime::VulkanRuntime()
{
    context = ContextBuilder()
                  .SetInstanceExtensions({VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"})
                  .EnableValidationLayer()
                  .SetInstanceLayers({"VK_LAYER_KHRONOS_validation"})
                  .SetDeviceExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
                  .Build();
}

VulkanRuntime::~VulkanRuntime()
{
    context.reset();
}

IntrusivePtr<Context> VulkanRuntime::GetContext()
{
    return context;
}

IntrusivePtr<RenderPass> VulkanRuntime::CreateRenderPass(IntrusivePtr<Graph> graph)
{
    return new VulkanRenderPass(context, graph);
}

IntrusivePtr<Pipeline> VulkanRuntime::CreatePipeline(IntrusivePtr<RenderPass> renderPass, std::string subPassName, PipelineStates pipelineStates)
{
    if (static_cast<VulkanRenderPass *>(renderPass.get())->GetSubPassIndex(subPassName) == -1)
    {
        spdlog::info("subPass: {} not exist in renderPass {}", subPassName, renderPass->GetGraph()->name);
        return nullptr;
    }
    return new VulkanPipeline(context, renderPass, subPassName, pipelineStates);
}

IntrusivePtr<Buffer> VulkanRuntime::CreateBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size)
{
    auto buffer = new VulkanBuffer(context);
    buffer->Allocate(type, memoryProperties, size);
    return buffer;
}

IntrusivePtr<Texture> VulkanRuntime::CreateTexture(TextureFormat format, Texture::UsageBits type, MemoryPropertyBits memoryProperties, Texture::Extent extent, Texture::Configuration config)
{
    auto texture = new VulkanTexture(context);
    texture->Allocate(format, type, memoryProperties, extent, config);
    return texture;
}

IntrusivePtr<RenderPassExecutor> VulkanRuntime::CreateRenderPassExecutor(IntrusivePtr<RenderPass> renderPass)
{
    auto rpe = new VulkanRenderPassExecutor(context, renderPass);

    return rpe;
}

IntrusivePtr<ResourceBindingState> VulkanRuntime::CreateResourceBindingState(IntrusivePtr<Pipeline> pipeline)
{
    return new VulkanResourceBindingState(context, pipeline);
}
