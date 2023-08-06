#pragma once

#include <Core/IntrusivePtr.h>
#include <RHI/VulkanRuntime/Context.h>

#include <vulkan/vulkan.h>

class VulkanTexture;
class VulkanTextureView : public IntrusiveCounter<VulkanTextureView>
{
public:
    VulkanTextureView(IntrusivePtr<Context> context);
    ~VulkanTextureView();

    bool Allocate(IntrusivePtr<VulkanTexture> parent, VkImageViewCreateInfo ci);
    VkImageView GetImageView()
    {
        return imageView;
    }

private:
    IntrusivePtr<Context> context;
    IntrusivePtr<VulkanTexture> parentTexture;
    VkImageView imageView = VK_NULL_HANDLE;
};