#pragma once

#include <string>

#include <RHI/Texture.h>
#include <RHI/RHIRuntime.h>

class KTXReader
{
public:
    KTXReader() = default;
    static IntrusivePtr<Texture> ReadFile(RHIRuntime *rhiRuntime, std::string file);
};