#include <Engine/Window.h>

#include <stdexcept>

#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CreateWin32Window(void *window)
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

Window::~Window()
{
    
}

void Window::Build(uint32_t width, uint32_t height)
{
    this->hwnd = CreateWin32Window(this);
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
