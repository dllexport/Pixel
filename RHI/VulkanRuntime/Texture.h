#pragma once

#include <RHI/Texture.h>
#include <RHI/VulkanRuntime/Context.h>

class VulkanTexture : public Texture
{
public:
    VulkanTexture(IntrusivePtr<Context> context);

private:
    friend class VulkanRuntime;
    IntrusivePtr<Context> context;
    VkImage image;
    VmaAllocation imageAllocation;
    VmaAllocationInfo imageAllocationInfo;

    virtual bool Allocate(TextureFormat format, UsageBits type, MemoryPropertyBits memoryProperties, Extent extent, Configuration config) override;
};