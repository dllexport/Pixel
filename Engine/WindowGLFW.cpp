#ifdef WINDOW_GLFW

#include <Engine/Window.h>

#include <stdexcept>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

void *CreateGLFWWindow(void *window, uint32_t width, uint32_t height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *glfwWindow = glfwCreateWindow(width, height, "Window Title", NULL, NULL);
    return glfwWindow;
}

Window::Window()
{
    glfwInit();
    uint32_t count;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);
    for (int i = 0; i < count; i++) {
        spdlog::info("{}", extensions[i]);
    }

    if (!glfwVulkanSupported())
    {
        throw std::runtime_error("!glfwVulkanSupported");
    }
}

Window::~Window()
{
}

void Window::Build(uint32_t width, uint32_t height)
{
    this->hwnd = CreateGLFWWindow(this, width, height);
}

bool Window::Stopped()
{
    return stopped;
}

void Window::Update()
{
}

#endif