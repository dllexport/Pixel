#pragma once

#include <Core/IntrusivePtr.h>
#include <RHI/ResourceHandle.h>
#include <RHI/TextureFormat.h>
#include <RHI/Memory.h>

class Texture : public ResourceHandle
{
public:
    enum Usage
    {
        IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001,
        IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002,
        IMAGE_USAGE_SAMPLED_BIT = 0x00000004,
        IMAGE_USAGE_STORAGE_BIT = 0x00000008,
        IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010,
        IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020,
        IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT = 0x00000040,
        IMAGE_USAGE_INPUT_ATTACHMENT_BIT = 0x00000080,
        _IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT = 0x00000200,
        IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR = 0x00000100,
        IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT = 0x00080000,
        IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI = 0x00040000,
        IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM = 0x00100000,
        IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM = 0x00200000,
        IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV = IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
        IMAGE_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };

    using UsageBits = uint32_t;

    struct Extent
    {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
    };

    enum Tiling
    {
        IMAGE_TILING_OPTIMAL = 0,
        IMAGE_TILING_LINEAR = 1,
    };

    struct Configuration
    {
        uint32_t mipLevels;
        uint32_t arrayLayers;
        uint32_t samples;
        Tiling tiling;

        // VkSharingMode sharingMode;
        // uint32_t queueFamilyIndexCount;
        // const uint32_t *pQueueFamilyIndices;
        // VkImageLayout initialLayout;
    };

    Texture() = default;
    virtual ~Texture() = default;

protected:
    virtual bool Allocate(TextureFormat format, UsageBits type, MemoryPropertyBits memoryProperties, Extent extent, Configuration config) = 0;
};