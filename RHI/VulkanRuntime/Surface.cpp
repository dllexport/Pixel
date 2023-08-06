#include <RHI/VulkanRuntime/Surface.h>

#include <stdexcept>

#include <Windows.h>
#include <vulkan/vulkan_win32.h>

Surface::Surface(IntrusivePtr<Context> context, void *handle) : context(context), handle(handle)
{
}

void Surface::Build()
{
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = reinterpret_cast<HWND>(this->handle);
    createInfo.hinstance = GetModuleHandle(nullptr);

    if (!handle)
    {
        throw std::runtime_error("Null surface handle");
    }

    if (vkCreateWin32SurfaceKHR(context->GetVkInstance(), &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface");
    }
}

VkSurfaceKHR Surface::GetSurface()
{
    return surface;
}