#include <RHI/VulkanRuntime/Runtime.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/ContextBuilder.h>

static IntrusivePtr<Context> context;

VulkanRuntime::VulkanRuntime()
{
    context = ContextBuilder()
        .SetInstanceExtensions({VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"})
        .EnableValidationLayer()
        .SetInstanceLayers({"VK_LAYER_KHRONOS_validation"})
        .SetDeviceExtensions({VK_KHR_SWAPCHAIN_EXTENSION_NAME})
        .Build();
}
