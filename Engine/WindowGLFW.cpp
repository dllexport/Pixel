#ifdef WINDOW_USE_GLFW

#include <Engine/Window.h>

#include <stdexcept>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

struct WindowCallbacks
{
    static void window_size_callback(GLFWwindow *window, int width, int height)
    {
        auto pWnd = (Window *)glfwGetWindowUserPointer(window);
        pWnd->resizeCallback((uint32_t)width, (uint32_t)height);
    }
};

void *CreateGLFWWindow(void *window, uint32_t width, uint32_t height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *glfwWindow = glfwCreateWindow(width, height, "Window Title", NULL, NULL);

    glfwSetWindowSizeCallback(glfwWindow, WindowCallbacks::window_size_callback);

    return glfwWindow;
}

Window::Window()
{
    glfwInit();

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
    glfwSetWindowUserPointer((GLFWwindow *)this->hwnd, this);
}

bool Window::Stopped()
{
    return glfwWindowShouldClose((GLFWwindow *)this->hwnd);
}

void Window::Update()
{
    glfwPollEvents();
}

#endif