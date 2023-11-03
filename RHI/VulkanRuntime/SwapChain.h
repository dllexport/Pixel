#pragma once

#include <RHI/SwapChain.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Surface.h>

#include <vulkan/vulkan.h>

class VulkanSwapChain : public SwapChain
{
public:
    VulkanSwapChain(IntrusivePtr<Context> context);
    virtual ~VulkanSwapChain() override;

    virtual uint32_t Acquire(uint32_t currentFrame) override;
    virtual bool Present(uint32_t index, uint32_t currentFrame) override;
    virtual uint32_t ImageSize() override
    {
        return this->swapChainTextures.size();
    }

    std::vector<IntrusivePtr<VulkanTexture>> &GetTextures()
    {
        return swapChainTextures;
    }

    std::vector<VkSemaphore> &GetImageAvailableSemaphores()
    {
        return imageAvailableSemaphores;
    }

    std::vector<VkSemaphore> &GetRenderFinishedSemaphores()
    {
        return renderFinishedSemaphores;
    }

    IntrusivePtr<Surface> GetSurface()
    {
        return surface;
    }

private:
    friend class SwapChainBuilder;
    friend class VulkanRenderGroup;
    IntrusivePtr<Context> context;

    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR format;
    VkFormat depthStencilFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    IntrusivePtr<Surface> surface;
    VkSwapchainKHR swapChain;
    std::vector<IntrusivePtr<VulkanTexture>> swapChainTextures;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;

    void InitSync(uint32_t imageSize);
};