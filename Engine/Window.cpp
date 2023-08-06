#include <Engine/Window.h>

#include <stdexcept>

#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CreateWin32Window()
{
    LPCSTR CLASS_NAME = {"Sample Window Class"};

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
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
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,

                               NULL,         // Parent window
                               NULL,         // Menu
                               wc.hInstance, // Instance handle
                               NULL          // Additional application data
    );

    if (!hwnd)
    {
        std::runtime_error("Failed to create window");
    }
    else
    {
        ShowWindow(hwnd, SW_NORMAL);
    }

    return hwnd;
}

Window::~Window()
{
    CloseWindow((HWND)hwnd);
}

void Window::Build(uint32_t width, uint32_t height)
{
    this->hwnd = CreateWin32Window();
}