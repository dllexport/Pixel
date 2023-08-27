#pragma once

#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Sampler.h>

// image is a pair of texture and textureView
struct VulkanImage
{
    IntrusivePtr<VulkanTexture> texture;
    IntrusivePtr<VulkanTextureView> textureView;
    IntrusivePtr<VulkanSampler> sampler;
};