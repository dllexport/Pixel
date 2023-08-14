#pragma once

#include <Core/IntrusivePtr.h>

class Window : public IntrusiveCounter<Window>
{
public:
    Window();
    ~Window();

    void Build(uint32_t width, uint32_t height);

    void *GetHandle()
    {
        return hwnd;
    }

    void Update();

    bool Stopped();

    void SetStopped()
    {
        stopped = true;
    }

    void RegisterResizeCallback(std::function<void(uint32_t, uint32_t)> fn)
    {
        this->resizeCallback = fn;
    }

private:
    friend class WindowCallbacks;
    bool stopped = false;
    bool focus = false;
    void *hwnd;

    std::function<void(uint32_t, uint32_t)> resizeCallback;
};