#include <RHI/VulkanRuntime/Runtime.h>

#include <spdlog/spdlog.h>

#include <RHI/ConstantBuffer.h>
#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/ContextBuilder.h>
#include <RHI/VulkanRuntime/RenderGroup.h>
#include <RHI/VulkanRuntime/RenderGroupExecutor.h>
#include <RHI/VulkanRuntime/GraphicsPipeline.h>
#include <RHI/VulkanRuntime/ComputePipeline.h>
#include <RHI/VulkanRuntime/Buffer.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/Sampler.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/ResourceBindingState.h>
#include <RHI/VulkanRuntime/SwapChain.h>
#include <RHI/VulkanRuntime/SwapChainBuilder.h>
#include <RHI/VulkanRuntime/AuxiliaryExecutor.h>

#ifdef WINDOW_USE_GLFW
#include <GLFW/glfw3.h>
#endif

VulkanRuntime::VulkanRuntime()
{
    std::vector<const char *> instanceExts;

#ifdef WINDOW_USE_GLFW
    glfwInit();

    uint32_t count;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);
    for (int i = 0; i < count; i++)
    {
        instanceExts.push_back(extensions[i]);
    }
#endif

    std::vector<const char *> enableLayers = {"VK_LAYER_KHRONOS_validation"};

// VK_LAYER_LUNARG_monitor not support on mac
#ifndef __APPLE__
    enableLayers.push_back("VK_LAYER_LUNARG_monitor");
#endif

    context = ContextBuilder()
                  .SetInstanceExtensions(std::move(instanceExts))
                  .EnableValidationLayer()
                  .SetInstanceLayers(std::move(enableLayers))
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

IntrusivePtr<RenderGroup> VulkanRuntime::CreateRenderGroup(IntrusivePtr<Graph> graph)
{
    auto ae = new VulkanAuxiliaryExecutor(context);
    return new VulkanRenderGroup(context, graph, ae);
}

IntrusivePtr<Pipeline> VulkanRuntime::CreatePipeline(IntrusivePtr<RenderGroup> renderGroup, std::string subPassName, PipelineStates pipelineStates)
{
    auto vrg = static_cast<VulkanRenderGroup *>(renderGroup.get());
    auto rp = vrg->GetRenderPass(subPassName);
    if (!rp)
    {
        spdlog::info("subPass: {} not exist in renderPass {}", subPassName, vrg->GetGraph()->GetName());
        return nullptr;
    }
    auto pipeline = new VulkanGraphicsPipeline(context, rp, subPassName, pipelineStates);
    pipeline->groupName = renderGroup->Name();
    return pipeline;
}

IntrusivePtr<Pipeline> VulkanRuntime::CreateComputePipeline(IntrusivePtr<RenderGroup> renderPass, std::string subPassName)
{
    return new VulkanComputePipeline(context, renderPass, subPassName);
}

IntrusivePtr<Buffer> VulkanRuntime::CreateBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size)
{
    if (memoryProperties & MemoryProperty::MEMORY_PROPERTY_HOST_LOCAL_BIT)
    {
        return new ConstantBuffer(size);
    }

    auto buffer = new VulkanBuffer(context);
    buffer->Allocate(type, memoryProperties, size);
    return buffer;
}

IntrusivePtr<MutableBuffer> VulkanRuntime::CreateMutableBuffer(Buffer::TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size)
{
    auto defaultBuffer = CreateBuffer(type, memoryProperties, size);
    auto bufferArray = new MutableBuffer(defaultBuffer);
    return bufferArray;
}

IntrusivePtr<Texture> VulkanRuntime::CreateTexture(TextureFormat format, Texture::UsageBits type, MemoryPropertyBits memoryProperties, Texture::Extent extent, Texture::Configuration config)
{
    auto texture = new VulkanTexture(context);
    texture->Allocate(format, type, memoryProperties, extent, config);
    return texture;
}

IntrusivePtr<Sampler> VulkanRuntime::CreateSampler(IntrusivePtr<Texture> texture, Sampler::Configuration config)
{
    auto sampler = new VulkanSampler(context, texture);
    sampler->Allocate(config);
    return sampler;
}

IntrusivePtr<RenderGroupExecutor> VulkanRuntime::CreateRenderGroupExecutor()
{
    auto rpe = new VulkanGroupExecutor(context);
    return rpe;
}

IntrusivePtr<ResourceBindingState> VulkanRuntime::CreateResourceBindingState(IntrusivePtr<Pipeline> pipeline)
{
    return new VulkanResourceBindingState(context, pipeline);
}

IntrusivePtr<SwapChain> VulkanRuntime::CreateSwapChain(void *handle, uint32_t width, uint32_t height, IntrusivePtr<SwapChain> oldSwapChain)
{
    auto osc = static_cast<VulkanSwapChain *>(oldSwapChain.get());
    auto builder = SwapChainBuilder(context)
                       .SetExtent(width, height)
                       .SetHandle(handle)
                       .SetPreferPresentMode(VK_PRESENT_MODE_FIFO_KHR)
                       .SetPreferFormat({VK_FORMAT_B8G8R8A8_UNORM});
    if (osc)
    {
        builder.SetSurface(osc->GetSurface())
            .SetOldSwapChain(osc);
    }
    return builder.Build();
}

IntrusivePtr<AuxiliaryExecutor> VulkanRuntime::CreateAuxiliaryExecutor()
{
    return new VulkanAuxiliaryExecutor(context);
}
