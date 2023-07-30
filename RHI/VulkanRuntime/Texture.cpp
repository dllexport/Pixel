#include <RHI/VulkanRuntime/Texture.h>

VkFormat TranslateTextureFormat(TextureFormat format)
{
    switch (format)
    {
    case TextureFormat::FORMAT_B8G8R8A8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::FORMAT_D32_SFLOAT_S8_UINT:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    default:
        return VK_FORMAT_UNDEFINED;
    }
}

VulkanTexture::VulkanTexture(IntrusivePtr<Context> context) : context(context)
{
}

bool VulkanTexture::Allocate(TextureFormat format, UsageBits type, MemoryPropertyBits memoryProperties, Extent extent, Configuration config)
{
    VkImageCreateInfo imageCI = {};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = TranslateTextureFormat(format);
    imageCI.extent.width = extent.width;
    imageCI.extent.height = extent.height;
    imageCI.extent.depth = extent.depth;
    imageCI.mipLevels = config.mipLevels;
    imageCI.arrayLayers = config.arrayLayers;
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCI.tiling = VkImageTiling(config.tiling);
    imageCI.usage = type;
    imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    auto allocator = context->GetVmaAllocator();
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.requiredFlags = memoryProperties;

    auto result = vmaCreateImage(allocator, &imageCI, &allocInfo, &image, &imageAllocation, &imageAllocationInfo);

    return result == VK_SUCCESS;
}
