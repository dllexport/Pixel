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

    static void window_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        auto pWnd = (Window *)glfwGetWindowUserPointer(window);

        Event::Type type;
        if (action == GLFW_PRESS)
            type = Event::KEY_DOWN;
        if (action == GLFW_RELEASE)
            type = Event::KEY_UP;
        if (action == GLFW_REPEAT)
            type = Event::KEY_REPEAT;

        spdlog::info("action {}, code {}, mods {}", action, scancode, mods);
        Event event = {};
        event.type = type;
        event.keyCode = scancode;
        pWnd->eventCallback(event);
    }

    static void window_mouse_callback(GLFWwindow *window, int button, int action, int mods)
    {
        auto pWnd = (Window *)glfwGetWindowUserPointer(window);
        double x;
        double y;
        glfwGetCursorPos(window, &x, &y);

        auto type = (action == GLFW_PRESS) ? Event::MOUSE_DOWN : Event::MOUSE_UP;

        Event event = {};
        event.type = type;
        event.mouseX = x;
        event.mouseY = y;
        pWnd->eventCallback(event);
    }

    static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
    {
        auto pWnd = (Window *)glfwGetWindowUserPointer(window);
        Event event = {};
        event.type = Event::MOUSE_MOVE;
        event.mouseX = xpos;
        event.mouseY = ypos;
        pWnd->eventCallback(event);
    }

    static void cursor_enter_callback(GLFWwindow *window, int entered)
    {
        auto pWnd = (Window *)glfwGetWindowUserPointer(window);
        pWnd->focus = !!entered;
        auto type = !!entered ? Event::WINDOW_FOCUS_IN : Event::WINDOW_FOCUS_OUT;
        Event event = {};
        event.type = type;
        pWnd->eventCallback(event);
    }
};

void *CreateGLFWWindow(void *window, uint32_t width, uint32_t height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *glfwWindow = glfwCreateWindow(width, height, "Window Title", NULL, NULL);

    glfwSetWindowSizeCallback(glfwWindow, WindowCallbacks::window_size_callback);
    glfwSetKeyCallback(glfwWindow, WindowCallbacks::window_key_callback);
    glfwSetMouseButtonCallback(glfwWindow, WindowCallbacks::window_mouse_callback);
    glfwSetCursorPosCallback(glfwWindow, WindowCallbacks::cursor_position_callback);
    glfwSetCursorEnterCallback(glfwWindow, WindowCallbacks::cursor_enter_callback);

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