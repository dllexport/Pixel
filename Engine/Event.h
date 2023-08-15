#pragma once

#include <cstdint>

class Event
{
public:
    enum Type : uint16_t
    {
        KEY_REPEAT = 1,
        KEY_DOWN = 1 << 2,
        KEY_UP = 1 << 3,
        MOUSE_DOWN = 1 << 4,
        MOUSE_UP = 1 << 5,
        MOUSE_MOVE = 1 << 6,
        WINDOW_FOCUS_IN = 1 << 7,
        WINDOW_FOCUS_OUT = 1 << 8,
        FRAME = 1 << 9
    };

    using TypeBits = uint16_t;
    TypeBits type;

    union
    {
        char keyCode;
        struct
        {
            uint16_t mouseX;
            uint16_t mouseY;
        };
    };
};

class IOState
{
public:
    IOState()
    {
        memset(keyboards, 0, sizeof(keyboards));
        memset(mouse, 0, sizeof(mouse));
        mouseX = 0;
        mouseY = 0;
    }

    char keyboards[104];
    char mouse[3];
    uint32_t mouseX;
    uint32_t mouseY;
};