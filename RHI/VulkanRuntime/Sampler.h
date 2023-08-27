#pragma once

#include <unordered_set>

#include <RHI/Sampler.h>
#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>

class VulkanSampler : public Sampler
{
public:
    VulkanSampler(IntrusivePtr<Context> context, IntrusivePtr<Texture> texture);
    virtual ~VulkanSampler() override;
    virtual bool Allocate(Configuration config) override;

    VkSampler GetSampler()
    {
        return sampler;
    }

    IntrusivePtr<VulkanTexture> GetTexture()
    {
        return boost::static_pointer_cast<VulkanTexture>(this->texture);
    }

    void StoreTextureView(IntrusivePtr<VulkanTextureView> textureView)
    {
        this->textureViews.insert(textureView);
    }

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;
    std::unordered_set<IntrusivePtr<VulkanTextureView>> textureViews;
    VkSampler sampler;
};