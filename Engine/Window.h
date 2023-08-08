#pragma once

#include <Core/IntrusivePtr.h>

class Window : public IntrusiveCounter<Window>
{
public:
    Window() = default;
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

private:
    bool stopped = false;
    void *hwnd;
};