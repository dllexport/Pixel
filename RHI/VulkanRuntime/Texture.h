#pragma once

#include <unordered_set>

#include <RHI/Texture.h>
#include <RHI/VulkanRuntime/Context.h>

inline TextureFormat VkFormatToGeneral(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_D16_UNORM:
        return TextureFormat::FORMAT_D16_UNORM;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return TextureFormat::FORMAT_R16G16B16A16_SFLOAT;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return TextureFormat::FORMAT_B8G8R8A8_UNORM;
    default:
        return TextureFormat::NONE;
    }
}

inline VkFormat TranslateTextureFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::FORMAT_D16_UNORM:
        return VK_FORMAT_D16_UNORM;
    case TextureFormat::FORMAT_B8G8R8A8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::FORMAT_R16G16B16A16_SFLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}

class VulkanTextureView;
class VulkanTexture : public Texture
{
public:
    VulkanTexture(IntrusivePtr<Context> context);
    virtual ~VulkanTexture() override;
    virtual bool Allocate(TextureFormat format, UsageBits type, MemoryPropertyBits memoryProperties, Extent extent, Configuration config) override;

    VkImage GetImage();

    IntrusivePtr<VulkanTextureView> CreateTextureView(VkImageViewCreateInfo ci);

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;
    VkImage image;
    VmaAllocation imageAllocation;
    VmaAllocationInfo imageAllocationInfo;
};