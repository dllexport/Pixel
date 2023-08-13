#include <RHI/VulkanRuntime/Surface.h>

#include <stdexcept>

#ifdef WINDOW_GLFW
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#else
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

Surface::Surface(IntrusivePtr<Context> context, void *handle) : context(context), handle(handle)
{
}

void Surface::Build()
{
#ifdef WINDOW_GLFW
    VkResult err = glfwCreateWindowSurface(context->GetVkInstance(), (GLFWwindow *)handle, nullptr, &surface);
    if (err)
    {
        throw std::runtime_error("failed to create window surface");
    }
#else
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
#endif
}

VkSurfaceKHR Surface::GetSurface()
{
    return surface;
}