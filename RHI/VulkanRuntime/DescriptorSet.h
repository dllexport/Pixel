#pragma once

#include <Core/IntrusivePtr.h>

#include <RHI/ResourceHandle.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Pipeline.h>

#include <vulkan/vulkan.h>

class VulkanDescriptorSet : public IntrusiveCounter<VulkanDescriptorSet>
{
public:
    VulkanDescriptorSet(IntrusivePtr<Context> context, IntrusivePtr<Pipeline> pipeline);
    ~VulkanDescriptorSet();

    void Bind(IntrusivePtr<ResourceHandle> resource);
    void Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource);
    void Bind(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources);

    void AllocateDescriptorPool(IntrusivePtr<Pipeline> &pipeline);

    // pipeline's desc layout may contains multiple sets
    // allocate descriptorSets for frame overlap
    void AllocateExtraDescriptorSets(uint32_t frameIndex);
    void WriteDescriptor(uint32_t frameIndex, uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> &resource);
    void WriteDescriptor(uint32_t frameIndex, uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> &resources);

    void WriteDescriptorBuffer(uint32_t frameIndex, uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> &resources);
    void WriteDescriptorSampler(uint32_t frameIndex, uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> &resources);

    // bind internal resources
    void BindInternal(uint32_t frameIndex, uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource);

    void Copy(uint32_t targetFrameIndex, uint32_t set, uint32_t binding);

    IntrusivePtr<Context> context;
    IntrusivePtr<Pipeline> pipeline;

    VkDescriptorPool descriptorPool;

    // sets -> bindings -> resource handles
    using ResourceHandleMap = std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<IntrusivePtr<ResourceHandle>>>>;

    struct FrameDescriptor
    {
        std::vector<VkDescriptorSet> descriptorSets;
        ResourceHandleMap resourceHandlesMaps;
    };

    std::unordered_map<uint32_t, FrameDescriptor> frameDescriptor;
    IntrusivePtr<ResourceHandle> constantBuffer;
};