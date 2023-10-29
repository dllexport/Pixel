#pragma once

#include <Core/IntrusivePtr.h>

#include <RHI/ResourceHandle.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/GraphicsPipeline.h>

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
    void Bind(uint32_t frameIndex, uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource, bool internal);

    void Copy(uint32_t targetFrameIndex, uint32_t set, uint32_t binding);

    // clear all internal resource in resourceMap
    void ClearInternal();

    IntrusivePtr<Context> context;
    IntrusivePtr<Pipeline> pipeline;

    VkDescriptorPool descriptorPool;

    struct ResourceHandleMeta
    {
        std::vector<IntrusivePtr<ResourceHandle>> resourceHandles;
        bool internal;

        bool Empty()
        {
            return resourceHandles.empty();
        }
    };
    // sets -> bindings -> resource handles
    using ResourceHandleMap = std::unordered_map<uint32_t, std::unordered_map<uint32_t, ResourceHandleMeta>>;

    struct FrameDescriptor
    {
        // different set of descriptors may be bind
        std::vector<VkDescriptorSet> descriptorSets;
        // hold resource's ref
        ResourceHandleMap resourceHandlesMaps;
    };

    ResourceHandleMap &GetResourceHandlesMap(uint32_t frameIndex)
    {
        return this->frameDescriptor[frameIndex].resourceHandlesMaps;
    }

    std::unordered_map<uint32_t, FrameDescriptor> frameDescriptor;
    IntrusivePtr<ResourceHandle> constantBuffer;
};