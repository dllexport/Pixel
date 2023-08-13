#pragma once

#include <unordered_map>
#include <unordered_set>

#include <RHI/ResourceBindingState.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Buffer.h>

class VulkanResourceBindingState : public ResourceBindingState
{
public:
    VulkanResourceBindingState(IntrusivePtr<Context> context, IntrusivePtr<Pipeline> pipeline);
    virtual ~VulkanResourceBindingState() override;
    virtual void Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource) override;
    virtual void Bind(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources) override;

    IntrusivePtr<VulkanBuffer> GetVertexBuffer()
    {
        return static_cast<VulkanBuffer *>(this->vertexBuffer.get());
    }

    IntrusivePtr<VulkanBuffer> GetIndexBuffer()
    {
        return static_cast<VulkanBuffer *>(this->indexBuffer.get());
    }

    std::vector<VkDescriptorSet> &GetDescriptorSets()
    {
        return descriptorSets;
    }

private:
    IntrusivePtr<Context> context;

    std::vector<VkDescriptorSet> descriptorSets;
    VkDescriptorPool descriptorPool;

    // sets -> bindings -> resource handles
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::unordered_set<IntrusivePtr<ResourceHandle>>>> resourceHandlesMap;

    void AllocateDescriptorPool(IntrusivePtr<Pipeline> pipeline);

    // pipeline's desc layout may contains multiple sets
    void AllocateDescriptorSets();
    void WriteDescriptor(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource);
    void WriteDescriptor(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources);
};