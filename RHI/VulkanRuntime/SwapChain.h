#pragma once

#include <RHI/SwapChain.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Surface.h>

class VulkanSwapChain : public SwapChain
{
public:
    VulkanSwapChain(IntrusivePtr<Context> context);
    virtual ~VulkanSwapChain() override;

private:
    friend class SwapChainBuilder;
    IntrusivePtr<Context> context;

    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR format;
    VkFormat depthStencilFormat;
    VkPresentModeKHR presentMode;

    IntrusivePtr<Surface> surface;
    VkSwapchainKHR swapChain;
    std::vector<IntrusivePtr<VulkanTexture>> swapChainTextures;
    std::vector<IntrusivePtr<VulkanTextureView>> swapChainTextureViews;
};