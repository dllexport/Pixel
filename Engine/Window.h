#pragma once

#include <Core/IntrusivePtr.h>

class Window : public IntrusiveCounter<Window>
{
public:
    Window();
    ~Window();

    void Build(uint32_t width, uint32_t height);

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
    friend class Renderer;

    bool stopped = false;
    bool focus = false;
    void *hwnd;

    std::function<void(uint32_t, uint32_t)> resizeCallback;
};