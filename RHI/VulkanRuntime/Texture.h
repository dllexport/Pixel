#pragma once

#include <unordered_set>

#include <RHI/Texture.h>
#include <RHI/VulkanRuntime/Context.h>

inline TextureFormat VkFormatToGeneralFormat(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_B8G8R8A8_SRGB:
        return TextureFormat::FORMAT_B8G8R8A8_SRGB;
    case VK_FORMAT_D16_UNORM:
        return TextureFormat::FORMAT_D16_UNORM;
    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return TextureFormat::FORMAT_R16G16B16A16_SFLOAT;
    case VK_FORMAT_B8G8R8A8_UNORM:
        return TextureFormat::FORMAT_B8G8R8A8_UNORM;
    default:
        return TextureFormat::FORMAT_NONE;
    }
}

inline VkFormat GeneralFormatToVkFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::FORMAT_B8G8R8A8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;
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
    VulkanTexture(IntrusivePtr<Context> context, VkImage image);
    VulkanTexture(IntrusivePtr<Context> context);
    virtual ~VulkanTexture() override;
    virtual bool Allocate(TextureFormat format, UsageBits type, MemoryPropertyBits memoryProperties, Extent extent, Configuration config) override;

    void Assign(VkImage image, VkFormat format)
    {
        this->image = image;
        this->format = format;
    }

    VkImage GetImage();

    IntrusivePtr<VulkanTextureView> CreateTextureView(VkImageViewCreateInfo ci);

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;
    VkImage image;
    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo imageAllocationInfo;
    VkFormat format;

    bool IsExternal()
    {
        return image && !imageAllocation;
    }
};