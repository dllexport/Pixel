#pragma once

#include <unordered_map>
#include <unordered_set>

#include <RHI/ResourceBindingState.h>
#include <RHI/MutableBuffer.h>

#include <RHI/VulkanRuntime/Context.h>
#include <RHI/VulkanRuntime/Buffer.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/DescriptorSet.h>

class VulkanResourceBindingState : public ResourceBindingState
{
public:
    VulkanResourceBindingState(IntrusivePtr<Context> context, IntrusivePtr<Pipeline> pipeline);
    virtual ~VulkanResourceBindingState() override;
    virtual void Bind(IntrusivePtr<ResourceHandle> resource) override;
    virtual void Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource) override;
    virtual void Bind(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources) override;

    IntrusivePtr<VulkanBuffer> GetVertexBuffer(uint32_t frameIndex)
    {
        if (!vertexBuffer)
            return nullptr;

        if (this->vertexBuffer->type == ResourceHandle::BUFFER_ARRAY)
        {
            auto mutableMuffer = static_cast<MutableBuffer *>(this->vertexBuffer.get());
            return static_cast<VulkanBuffer *>(mutableMuffer->GetBuffer(frameIndex).get());
        }
        else
        {
            return static_cast<VulkanBuffer *>(this->vertexBuffer.get());
        }
    }

    IntrusivePtr<VulkanBuffer> GetIndexBuffer(uint32_t frameIndex)
    {
        if (!indexBuffer)
            return nullptr;
        if (this->indexBuffer->type == ResourceHandle::BUFFER_ARRAY)
        {
            auto mutableMuffer = static_cast<MutableBuffer *>(this->indexBuffer.get());
            return static_cast<VulkanBuffer *>(mutableMuffer->GetBuffer(frameIndex).get());
        }
        else
        {
            return static_cast<VulkanBuffer *>(this->indexBuffer.get());
        }
    }

    std::vector<VkDescriptorSet> &GetDescriptorSets(uint32_t frameIndex)
    {
        return descriptorSet->frameDescriptor[frameIndex].descriptorSets;
    }

    IntrusivePtr<ResourceHandle> &GetConstantBuffer()
    {
        return descriptorSet->constantBuffer;
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

    IntrusivePtr<VulkanDescriptorSet> &GetDescriptorSet()
    {
        return descriptorSet;
    }

    // bind internal resources
    void BindInternal(uint32_t frameIndex, uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource);

    // Copy constant resource only
    // if resource will be written, bind extra frame related resource instead
    void Copy(uint32_t targetFrameIndex, uint32_t set, uint32_t binding);

private:
    IntrusivePtr<Context> context;

    IntrusivePtr<VulkanDescriptorSet> descriptorSet;

    std::vector<UpdateRequest> updateRequests;
};