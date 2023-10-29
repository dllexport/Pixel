#pragma once

#include <vulkan/vulkan.h>
#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/SwapChain.h>
#include <RHI/VulkanRuntime/Surface.h>

class SwapChainBuilder
{
public:
    SwapChainBuilder(IntrusivePtr<Context> &context);
    SwapChainBuilder &SetHandle(void *windowHandle);
    SwapChainBuilder &SetExtent(uint32_t width, uint32_t height);
    SwapChainBuilder &SetPreferFormat(VkSurfaceFormatKHR format);
    SwapChainBuilder &SetPreferDepthStencilFormat(VkFormat format);
    SwapChainBuilder &SetPreferPresentMode(VkPresentModeKHR mode);
    SwapChainBuilder &SetBufferCount(uint32_t count);
    SwapChainBuilder &SetSurface(IntrusivePtr<Surface> surface);
    SwapChainBuilder &SetOldSwapChain(IntrusivePtr<VulkanSwapChain> oldSwapChain);
    IntrusivePtr<VulkanSwapChain> Build();

private:
    VkSurfaceCapabilitiesKHR capabilities;
    VkExtent2D windowExtent;
    IntrusivePtr<VulkanSwapChain> newSwapChain;
    IntrusivePtr<VulkanSwapChain> oldSwapChain;
    IntrusivePtr<Context> context;
    VkSurfaceFormatKHR preferFormat;
    VkFormat preferDepthStencilFormat = VK_FORMAT_UNDEFINED;
    VkPresentModeKHR preferPresentMode;
    uint32_t bufferCount = 2;
    IntrusivePtr<Surface> surface;

    void *windowHandle;

    void BuildSurface();
    void BuildSwapChainProperties();
    void BuildSwapChain();
    void ResolveDepthStencilFormat();
};