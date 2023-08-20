#pragma once

#include <unordered_map>
#include <unordered_set>

#include <RHI/ResourceBindingState.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Buffer.h>
#include <RHI/VulkanRuntime/Texture.h>

class VulkanResourceBindingState : public ResourceBindingState
{
public:
    VulkanResourceBindingState(IntrusivePtr<Context> context, IntrusivePtr<Pipeline> pipeline);
    virtual ~VulkanResourceBindingState() override;
    virtual void Bind(IntrusivePtr<ResourceHandle> resource) override;
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

    IntrusivePtr<ResourceHandle> &GetConstantBuffer()
    {
        return constantBuffer;
    }

    VkIndexType GetIndexType()
    {
        switch (indexType)
        {
        case INDEX_TYPE_UINT16:
            return VK_INDEX_TYPE_UINT16;
        case INDEX_TYPE_UINT32:
            return VK_INDEX_TYPE_UINT32;
        case INDEX_TYPE_UINT8:
            return VK_INDEX_TYPE_UINT8_EXT;
        default:
            return VK_INDEX_TYPE_NONE_KHR;
        }
    }

    struct UpdateRequest
    {
        IntrusivePtr<VulkanTexture> texutre;
        VkImageAspectFlags aspectMask;
        VkImageLayout oldLayout;
        VkImageLayout newLayout;
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
    };

    std::vector<UpdateRequest> &GetUpdateRequests()
    {
        return updateRequests;
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

    void WriteDescriptorBuffer(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources);
    void WriteDescriptorSampler(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources);

    IntrusivePtr<ResourceHandle> constantBuffer;

    std::vector<UpdateRequest> updateRequests;
};