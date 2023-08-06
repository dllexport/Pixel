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
    SwapChainBuilder &SetPreferFormat(VkSurfaceFormatKHR format);
    SwapChainBuilder &SetPreferDepthStencilFormat(VkFormat format);
    SwapChainBuilder &SetPreferPresentMode(VkPresentModeKHR mode);
    SwapChainBuilder &SetBufferCount(uint32_t count);
    SwapChainBuilder &SetOldSwapChain(IntrusivePtr<VulkanSwapChain> oldSwapChain);
    IntrusivePtr<VulkanSwapChain> Build();

private:
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