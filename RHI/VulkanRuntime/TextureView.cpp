#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Texture.h>

VulkanTextureView::VulkanTextureView(IntrusivePtr<Context> context) : context(context)
{
}

bool VulkanTextureView::Allocate(IntrusivePtr<VulkanTexture> parent, VkImageViewCreateInfo ci)
{
    this->parentTexture = parent;
    return vkCreateImageView(context->GetVkDevice(), &ci, nullptr, &imageView) == VK_SUCCESS;
}

VulkanTextureView::~VulkanTextureView()
{
    vkDestroyImageView(context->GetVkDevice(), imageView, nullptr);
}