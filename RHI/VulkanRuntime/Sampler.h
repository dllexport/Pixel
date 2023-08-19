#pragma once

#include <unordered_set>

#include <RHI/Sampler.h>
#include <RHI/VulkanRuntime/Context.h>

class VulkanSampler : public Sampler
{
public:
    VulkanSampler(IntrusivePtr<Context> context, IntrusivePtr<Texture> texture);
    virtual ~VulkanSampler() override;
    virtual bool Allocate(Configuration config) override;

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;
    VkSampler sampler;
};