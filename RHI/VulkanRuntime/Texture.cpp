#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>

VulkanTexture::VulkanTexture(IntrusivePtr<Context> context) : context(context)
{
}

VulkanTexture::~VulkanTexture()
{
    if (mappedData)
    {
        vmaUnmapMemory(context->GetVmaAllocator(), imageAllocation);
    }

    if (!IsExternal())
    {
        vmaDestroyImage(context->GetVmaAllocator(), image, imageAllocation);
    }
}

VkImage VulkanTexture::GetImage()
{
    return image;
}

bool VulkanTexture::Allocate(TextureFormat format, UsageBits type, MemoryPropertyBits memoryProperties, Extent extent, Configuration config)
{
    if (this->image)
        return true;

    VkImageCreateInfo imageCI = {};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = GeneralFormatToVkFormat(format);
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

    this->format = format;
    this->mimapLevel = config.mipLevels;
    this->layers = config.arrayLayers;
    this->samples = config.samples;
    this->extent.width = imageCI.extent.width;
    this->extent.height = imageCI.extent.height;
    this->extent.depth = imageCI.extent.depth;

    return result == VK_SUCCESS;
}

IntrusivePtr<VulkanTextureView> VulkanTexture::CreateTextureView(VkImageViewCreateInfo ci)
{

    IntrusivePtr<VulkanTextureView> view = new VulkanTextureView(context);
    view->Allocate(this, ci);
    return view;
}

void *VulkanTexture::Map()
{
    if (!mappedData)
    {
        auto result = vmaMapMemory(context->GetVmaAllocator(), imageAllocation, &mappedData);
        if (result != VK_SUCCESS)
        {
            return nullptr;
        }
    }

    return (char *)mappedData;
}

void VulkanTexture::Assign(VkImage image, VkFormat format)
{
    this->image = image;
    this->format = VkFormatToGeneralFormat(format);
}

VkFormat VulkanTexture::GetFormat()
{
    return GeneralFormatToVkFormat(format);
}

VkImageSubresourceRange VulkanTexture::GetImageSubResourceRange(VkImageAspectFlags aspectMask)
{
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;
    return subresourceRange;
}

bool VulkanTexture::IsExternal()
{
    return image && !imageAllocation;
}

bool VulkanTexture::IsSwapChain()
{
    return isSwapChain;
}

void VulkanTexture::SetSwapChain()
{
    isSwapChain = true;
}