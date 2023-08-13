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
#include <RHI/VulkanRuntime/SwapChain.h>
#include <RHI/VulkanRuntime/SwapChainBuilder.h>

VulkanRuntime::VulkanRuntime()
{
    std::vector<const char *> instanceExts = {VK_KHR_SURFACE_EXTENSION_NAME};
#ifdef _WIN32
    instanceExts.push_back("VK_KHR_win32_surface");
#elif __linux__
    instanceExts.push_back("VK_KHR_wayland_surface");
#endif

    context = ContextBuilder()
                  .SetInstanceExtensions(std::move(instanceExts))
                  .EnableValidationLayer()
                  .SetInstanceLayers({"VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_monitor"})
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

IntrusivePtr<RenderPassExecutor> VulkanRuntime::CreateRenderPassExecutor()
{
    auto rpe = new VulkanRenderPassExecutor(context);
    return rpe;
}

IntrusivePtr<ResourceBindingState> VulkanRuntime::CreateResourceBindingState(IntrusivePtr<Pipeline> pipeline)
{
    return new VulkanResourceBindingState(context, pipeline);
}

IntrusivePtr<SwapChain> VulkanRuntime::CreateSwapChain(void *handle, uint32_t width, uint32_t height)
{
    return SwapChainBuilder(context)
        .SetExtent(width, height)
        .SetHandle(handle)
        .SetPreferPresentMode(VK_PRESENT_MODE_IMMEDIATE_KHR)
        .SetPreferFormat({VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
        .Build();
}
