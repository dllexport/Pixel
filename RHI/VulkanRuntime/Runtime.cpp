#include <RHI/VulkanRuntime/Runtime.h>

#include <spdlog/spdlog.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/ContextBuilder.h>
#include <RHI/VulkanRuntime/RenderPass.h>
#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/Buffer.h>

VulkanRuntime::VulkanRuntime()
{
    context = ContextBuilder()
                  .SetInstanceExtensions({VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"})
                  .EnableValidationLayer()
                  .SetInstanceLayers({"VK_LAYER_KHRONOS_validation"})
                  .SetDeviceExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
                  .Build();
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

IntrusivePtr<Buffer> VulkanRuntime::CreateBuffer(Buffer::TypeBits type, Buffer::MemoryPropertyBits memoryProperties, uint32_t size)
{
    auto buffer = new VulkanBuffer(context);
    buffer->Allocate(type, memoryProperties, size);
    return buffer;
}
