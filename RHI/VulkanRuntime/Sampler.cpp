#include <RHI/VulkanRuntime/Sampler.h>

VulkanSampler::VulkanSampler(IntrusivePtr<Context> context, IntrusivePtr<Texture> texture) : Sampler(texture), context(context)
{
}

VulkanSampler::~VulkanSampler()
{
    vkDestroySampler(context->GetVkDevice(), sampler, nullptr);
}

bool VulkanSampler::Allocate(Configuration config)
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.maxAnisotropy = config.maxAnisotropy;
    samplerInfo.magFilter = VkFilter(config.magFilter);
    samplerInfo.minFilter = VkFilter(config.minFilter);
    samplerInfo.mipmapMode = VkSamplerMipmapMode(config.mipmapMode);
    samplerInfo.addressModeU = VkSamplerAddressMode(config.addressModeU);
    samplerInfo.addressModeV = VkSamplerAddressMode(config.addressModeV);
    samplerInfo.addressModeW = VkSamplerAddressMode(config.addressModeW);
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    auto result = vkCreateSampler(context->GetVkDevice(), &samplerInfo, nullptr, &sampler);
    return true;
}
