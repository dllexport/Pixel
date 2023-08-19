#include <RHI/VulkanRuntime/SwapChainBuilder.h>

#include <stdexcept>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>

SwapChainBuilder::SwapChainBuilder(IntrusivePtr<Context> &context) : context(context)
{
}

SwapChainBuilder &SwapChainBuilder::SetExtent(uint32_t width, uint32_t height)
{
    this->windowExtent = {width, height};
    return *this;
}

SwapChainBuilder &SwapChainBuilder::SetHandle(void *windowHandle)
{
    this->windowHandle = windowHandle;
    return *this;
}

SwapChainBuilder &SwapChainBuilder::SetPreferFormat(VkSurfaceFormatKHR format)
{
    this->preferFormat = format;
    return *this;
}

SwapChainBuilder &SwapChainBuilder::SetPreferDepthStencilFormat(VkFormat format)
{
    this->preferDepthStencilFormat = format;
    return *this;
}

SwapChainBuilder &SwapChainBuilder::SetPreferPresentMode(VkPresentModeKHR mode)
{
    this->preferPresentMode = mode;
    return *this;
}

SwapChainBuilder &SwapChainBuilder::SetBufferCount(uint32_t count)
{
    this->bufferCount = count;
    return *this;
}

SwapChainBuilder &SwapChainBuilder::SetOldSwapChain(IntrusivePtr<VulkanSwapChain> oldSwapChain)
{
    this->oldSwapChain = oldSwapChain;
    return *this;
}

void SwapChainBuilder::BuildSurface()
{
    this->surface = new Surface(context, this->windowHandle);
    this->surface->Build();
}

void SwapChainBuilder::BuildSwapChainProperties()
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->GetVkPhysicalDevice(), surface->GetSurface(), &capabilities);

    std::vector<VkSurfaceFormatKHR> formats;

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(context->GetVkPhysicalDevice(), surface->GetSurface(), &formatCount, nullptr);
    if (formatCount != 0)
    {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(context->GetVkPhysicalDevice(), surface->GetSurface(), &formatCount, formats.data());
    }

    std::vector<VkPresentModeKHR> presentModes;

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->GetVkPhysicalDevice(), surface->GetSurface(), &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(context->GetVkPhysicalDevice(), surface->GetSurface(), &presentModeCount, presentModes.data());
    }

    newSwapChain->format = formats[0];
    if (std::find_if(formats.begin(), formats.end(), [&](VkSurfaceFormatKHR &format)
                     { return (format.format == preferFormat.format) && (format.colorSpace == preferFormat.colorSpace); }) != formats.end())
    {
        newSwapChain->format = preferFormat;
    }

    newSwapChain->capabilities = capabilities;

    newSwapChain->presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (std::find(presentModes.begin(), presentModes.end(), preferPresentMode) != presentModes.end())
    {
        newSwapChain->presentMode = preferPresentMode;
    }

#ifndef NDEBUG
    static std::once_flag flag = {};
    std::call_once(flag, [&]()
                   {
    		spdlog::debug("available swapchain format:");
    		for (auto& format : formats)
    		{
    			spdlog::debug("{}:{}", vk::to_string(vk::Format(format.format)), vk::to_string(vk::ColorSpaceKHR(format.colorSpace)));
    		}

    		spdlog::debug("available present mode:");
    		for (auto& mode : presentModes)
    		{
    			spdlog::debug("{}", vk::to_string(vk::PresentModeKHR(mode)));
    		} });
#endif
}

void SwapChainBuilder::BuildSwapChain()
{
    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = surface->GetSurface();

    sci.minImageCount = std::max(capabilities.minImageCount, (uint32_t)2);
    sci.imageFormat = newSwapChain->format.format;
    sci.imageColorSpace = newSwapChain->format.colorSpace;
    sci.imageExtent = this->windowExtent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = newSwapChain->capabilities.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = newSwapChain->presentMode;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = VK_NULL_HANDLE;

    // save latest extent
    this->newSwapChain->extent = sci.imageExtent;

    // oldSwapChain must keep alive until creation
    if (oldSwapChain)
    {
        sci.oldSwapchain = oldSwapChain->swapChain;
    }

    VkSwapchainKHR swapChain;
    auto result = vkCreateSwapchainKHR(context->GetVkDevice(), &sci, nullptr, &swapChain);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain");
    }

    newSwapChain->swapChain = swapChain;
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(context->GetVkDevice(), swapChain, &imageCount, nullptr);

    std::vector<VkImage> images;
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(context->GetVkDevice(), swapChain, &imageCount, images.data());
    newSwapChain->swapChainTextures.resize(imageCount);
    newSwapChain->swapChainTextureViews.resize(imageCount);
    for (int i = 0; i < imageCount; i++)
    {
        IntrusivePtr<VulkanTexture> texture = new VulkanTexture(context);
        texture->Assign(images[i], newSwapChain->format.format);
        VkImageViewCreateInfo ci = {};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = newSwapChain->format.format;
        ci.subresourceRange = {};
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.baseMipLevel = 0;
        ci.subresourceRange.levelCount = 1;
        ci.subresourceRange.baseArrayLayer = 0;
        ci.subresourceRange.layerCount = 1;
        ci.image = texture->GetImage();

        auto textureView = texture->CreateTextureView(ci);

        newSwapChain->swapChainTextures[i] = texture;
        newSwapChain->swapChainTextureViews[i] = textureView;
    }
}

void SwapChainBuilder::ResolveDepthStencilFormat()
{
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> depthFormats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM};

    if (this->preferDepthStencilFormat != VK_FORMAT_UNDEFINED)
    {
        depthFormats.insert(depthFormats.begin(), preferDepthStencilFormat);
    }

    for (auto &format : depthFormats)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(context->GetVkPhysicalDevice(), format, &formatProps);
        // Format must support depth stencil attachment for optimal tiling
        if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            newSwapChain->depthStencilFormat = format;
            break;
        }
    }
}

IntrusivePtr<VulkanSwapChain> SwapChainBuilder::Build()
{
    this->newSwapChain = new VulkanSwapChain(context);
    BuildSurface();
    BuildSwapChainProperties();
    BuildSwapChain();
    ResolveDepthStencilFormat();
    newSwapChain->surface = this->surface;
    newSwapChain->InitSync(std::max(capabilities.minImageCount, (uint32_t)2));
    return this->newSwapChain;
}
