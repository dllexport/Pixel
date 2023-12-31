#ifndef WINDOW_USE_GLFW

#include <Engine/Window.h>

#include <stdexcept>

#include <Windows.h>

struct WindowCallbacks
{
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
        {
            IntrusivePtr<Window> window = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            DestroyWindow(hwnd);
            window->SetStopped();
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_SIZE:
        {
            IntrusivePtr<Window> window = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (!window) {
                break;
            }
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            window->resizeCallback(width, height);
            break;
        }
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};

HWND CreateWin32Window(void *window, uint32_t width, uint32_t height)
{
    LPCSTR CLASS_NAME = {"Sample Window Class"};

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowCallbacks::WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0,                            // Optional window styles.
                               CLASS_NAME,                   // Window class
                               {"Learn to Program Windows"}, // Window text
                               WS_OVERLAPPEDWINDOW,          // Window style

                               // Size and position
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               width,
                               height,

                               NULL,         // Parent window
                               NULL,         // Menu
                               wc.hInstance, // Instance handle
                               window        // Additional application data
    );

    if (!hwnd)
    {
        std::runtime_error("Failed to create window");
    }
    else
    {
        ShowWindow(hwnd, SW_NORMAL);
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    return hwnd;
}

Window::Window()
{
}

Window::~Window()
{
}

void Window::Build(uint32_t width, uint32_t height)
{
    this->hwnd = CreateWin32Window(this, width, height);
}

bool Window::Stopped()
{
    return stopped;
}

void Window::Update()
{
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

#endif