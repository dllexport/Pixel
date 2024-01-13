#pragma once
#include <Core/IntrusivePtr.h>
#include <RHI/ResourceHandle.h>
#include <RHI/Texture.h>
#include <RHI/MemoryProperties.h>

class Sampler : public ResourceHandle
{
public:
    Sampler(IntrusivePtr<Texture> texture) : ResourceHandle(ResourceHandleType::SAMPLER), texture(texture) {}
    virtual ~Sampler() = default;

    enum Filter
    {
        FILTER_NEAREST = 0,
        FILTER_LINEAR = 1,
        FILTER_CUBIC_EXT = 1000015000,
        FILTER_CUBIC_IMG = FILTER_CUBIC_EXT,
        FILTER_MAX_ENUM = 0x7FFFFFFF
    };

    enum MipmapMode
    {
        MIPMAP_MODE_NEAREST = 0,
        MIPMAP_MODE_LINEAR = 1,
        MIPMAP_MODE_MAX_ENUM = 0x7FFFFFFF
    };

    enum AddressMode
    {
        ADDRESS_MODE_REPEAT = 0,
        ADDRESS_MODE_MIRRORED_REPEAT = 1,
        ADDRESS_MODE_CLAMP_TO_EDGE = 2,
        ADDRESS_MODE_CLAMP_TO_BORDER = 3,
        ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE = 4,
        ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR = ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
        ADDRESS_MODE_MAX_ENUM = 0x7FFFFFFF
    };

    struct Configuration
    {
        Filter magFilter = FILTER_LINEAR;
        Filter minFilter = FILTER_LINEAR;
        MipmapMode mipmapMode = MIPMAP_MODE_LINEAR;
        AddressMode addressModeU = ADDRESS_MODE_CLAMP_TO_EDGE;
        AddressMode addressModeV = ADDRESS_MODE_CLAMP_TO_EDGE;
        AddressMode addressModeW = ADDRESS_MODE_CLAMP_TO_EDGE;
        float maxAnisotropy = 1.0f;
    };

    virtual bool Allocate(Configuration config) = 0;

protected:
    IntrusivePtr<Texture> texture;
    Configuration config;
};