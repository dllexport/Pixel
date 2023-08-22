#pragma once

#include <string>

#include <Engine/PixelEngine.h>
#include <RHI/Texture.h>

class KTXReader
{
public:
    static IntrusivePtr<Texture> ReadFile(PixelEngine *engine, std::string file);
};