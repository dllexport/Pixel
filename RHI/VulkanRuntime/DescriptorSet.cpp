#include <RHI/VulkanRuntime/DescriptorSet.h>

#include <stdexcept>

#include <RHI/VulkanRuntime/PipelineLayout.h>

#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/Buffer.h>
#include <RHI/VulkanRuntime/Sampler.h>
#include <RHI/ConstantBuffer.h>
#include <RHI/MutableBuffer.h>

VulkanDescriptorSet::VulkanDescriptorSet(IntrusivePtr<Context> context, IntrusivePtr<Pipeline> pipeline) : context(context), pipeline(pipeline)
{
    AllocateDescriptorPool(pipeline);
    AllocateExtraDescriptorSets(0);
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    vkDestroyDescriptorPool(context->GetVkDevice(), descriptorPool, nullptr);
    frameDescriptor.clear();
    constantBuffer.reset();
}

void VulkanDescriptorSet::AllocateExtraDescriptorSets(uint32_t frameIndex)
{
    auto vulkanPipeline = static_cast<VulkanPipeline *>(pipeline.get());
    auto layouts = vulkanPipeline->GetPipelineLayout()->GetSetsLayouts();

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.pSetLayouts = layouts.data();
    descriptorSetAllocateInfo.descriptorSetCount = uint32_t(layouts.size());

    auto &descriptorSets = frameDescriptor[frameIndex].descriptorSets;
    descriptorSets.resize(layouts.size());

    auto result = vkAllocateDescriptorSets(context->GetVkDevice(), &descriptorSetAllocateInfo, descriptorSets.data());
}

void VulkanDescriptorSet::Bind(IntrusivePtr<ResourceHandle> resource)
{
    this->constantBuffer = resource;
}

void VulkanDescriptorSet::Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource)
{
    Bind(set, binding, std::vector<IntrusivePtr<ResourceHandle>>{resource});
}

void VulkanDescriptorSet::Bind(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources)
{
    auto &resourceHandlesMaps = this->frameDescriptor[0].resourceHandlesMaps;
    resourceHandlesMaps[set][binding] = {resources, false};
    WriteDescriptor(0, set, binding, resources);
}

void VulkanDescriptorSet::Bind(uint32_t frameIndex, uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource, bool internal)
{
    auto &resourceHandlesMaps = this->frameDescriptor[frameIndex].resourceHandlesMaps;
    resourceHandlesMaps[set][binding] = {{resource}, internal};
    WriteDescriptor(frameIndex, set, binding, resource);
}

void VulkanDescriptorSet::ClearInternal()
{
    for (auto &[index, fd] : this->frameDescriptor)
    {
        for (auto &[set, bindings] : fd.resourceHandlesMaps)
        {
            for (auto &[binding, desc] : bindings)
            {
                if (desc.internal)
                    desc.resourceHandles.clear();
            }
        }
    }
}

void VulkanDescriptorSet::AllocateDescriptorPool(IntrusivePtr<Pipeline> &pipeline)
{
    auto vulkanPipeline = static_cast<VulkanPipeline *>(pipeline.get());
    auto &bindingsInSets = vulkanPipeline->GetPipelineLayout()->GetBindingsInSets();

    std::vector<VkDescriptorPoolSize> poolSizes;

    for (auto bindings : bindingsInSets)
    {
        if (bindings.empty())
        {
            continue;
        }
        for (auto binding : bindings)
        {
            VkDescriptorPoolSize descriptorPoolSize = {};
            descriptorPoolSize.type = binding.descriptorType;
            descriptorPoolSize.descriptorCount = binding.descriptorCount * 1024;
            poolSizes.push_back(descriptorPoolSize);
        }
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = uint32_t(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = 1024;

    auto result = vkCreateDescriptorPool(context->GetVkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool);
}

// resources should have same type
static void WriteDescriptorValidate(std::vector<IntrusivePtr<ResourceHandle>> &resources)
{
#ifndef NDEBUG
    bool bufferType = false;
    bool bufferArrayType = false;
    bool textureType = false;
    bool samplerType = false;
    for (auto resource : resources)
    {
        if (boost::dynamic_pointer_cast<VulkanBuffer>(resource))
        {
            bufferType = true;
        }
        if (boost::dynamic_pointer_cast<MutableBuffer>(resource))
        {
            bufferArrayType = true;
        }
        if (boost::dynamic_pointer_cast<VulkanTexture>(resource))
        {
            textureType = true;
        }
        if (boost::dynamic_pointer_cast<VulkanSampler>(resource))
        {
            samplerType = true;
        }
    }

    uint8_t typeCounter = 0;

    if (bufferType)
        typeCounter += 1;
    if (bufferArrayType)
        typeCounter += 1;
    if (textureType)
        typeCounter += 1;
    if (samplerType)
        typeCounter += 1;

    assert(typeCounter == 1);
#endif
}

void VulkanDescriptorSet::WriteDescriptor(uint32_t frameIndex, uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> &resource)
{
    std::vector<IntrusivePtr<ResourceHandle>> resourceHandles{resource};
    WriteDescriptor(frameIndex, set, binding, resourceHandles);
}

void VulkanDescriptorSet::WriteDescriptor(uint32_t frameIndex, uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> &resources)
{
    WriteDescriptorValidate(resources);

    if (frameDescriptor[frameIndex].descriptorSets.empty())
    {
        AllocateExtraDescriptorSets(frameIndex);
    }

    switch (resources[0]->type)
    {
    case ResourceHandle::BUFFER_ARRAY:
    case ResourceHandle::BUFFER:
    {
        WriteDescriptorBuffer(frameIndex, set, binding, resources);
        break;
    }
    case ResourceHandle::TEXTURE:
    {
        throw std::runtime_error("no impl");
    }
    case ResourceHandle::SAMPLER:
    {
        WriteDescriptorSampler(frameIndex, set, binding, resources);
        break;
    }
    }
}

void VulkanDescriptorSet::WriteDescriptorBuffer(uint32_t frameIndex, uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> &resources)
{
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;

    std::vector<VkDescriptorBufferInfo> bufferInfos(resources.size());

    for (int i = 0; i < resources.size(); i++)
    {
        VulkanBuffer *buffer = nullptr;
        if (resources[i]->type == ResourceHandle::BUFFER_ARRAY)
        {
            auto bufferArray = dynamic_cast<MutableBuffer *>(resources[i].get());
            buffer = static_cast<VulkanBuffer *>(bufferArray->GetBuffer(frameIndex).get());
        }
        else
        {
            // if resource is BUFFER
            buffer = static_cast<VulkanBuffer *>(resources[i].get());
        }

        VkDescriptorBufferInfo bufferInfo = {
            .buffer = buffer->GetBuffer(),
            .offset = 0,
            .range = VK_WHOLE_SIZE};
        bufferInfos[i] = bufferInfo;
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = frameDescriptor[frameIndex].descriptorSets[set];
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.pBufferInfo = bufferInfos.data();
    writeDescriptorSet.descriptorCount = (uint32_t)bufferInfos.size();

    writeDescriptorSets.push_back(writeDescriptorSet);

    vkUpdateDescriptorSets(context->GetVkDevice(), (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanDescriptorSet::WriteDescriptorSampler(uint32_t frameIndex, uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> &resources)
{
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;

    std::vector<VkDescriptorImageInfo> samplerInfos(resources.size());

    for (int i = 0; i < resources.size(); i++)
    {
        auto sampler = dynamic_cast<VulkanSampler *>(resources[i].get());
        auto texture = sampler->GetTexture();
        // TODO
        // UpdateRequest ur{
        //     texture, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT};
        // this->updateRequests.push_back(ur);
        VkImageViewCreateInfo ci = {};
        ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ci.format = texture->GetFormat();
        ci.subresourceRange = {};
        ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ci.subresourceRange.baseMipLevel = 0;
        ci.subresourceRange.levelCount = texture->LevelCount();
        ci.subresourceRange.baseArrayLayer = 0;
        ci.subresourceRange.layerCount = texture->LayerCount();
        ci.image = texture->GetImage();
        auto textureView = texture->CreateTextureView(ci);
        sampler->StoreTextureView(textureView);
        VkDescriptorImageInfo samplerInfo = {
            .sampler = sampler->GetSampler(),
            .imageView = textureView->GetImageView(),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        samplerInfos[i] = samplerInfo;
    }

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = frameDescriptor[frameIndex].descriptorSets[set];
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.pImageInfo = samplerInfos.data();
    writeDescriptorSet.descriptorCount = (uint32_t)samplerInfos.size();

    writeDescriptorSets.push_back(writeDescriptorSet);

    vkUpdateDescriptorSets(context->GetVkDevice(), (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanDescriptorSet::Copy(uint32_t targetFrameIndex, uint32_t set, uint32_t binding)
{
    auto &originalResourceMeta = frameDescriptor[0].resourceHandlesMaps[set][binding];
    auto &originalResource = originalResourceMeta.resourceHandles[0];
    switch (originalResource->type)
    {
    case ResourceHandle::ResourceHandleType::SAMPLER:
    case ResourceHandle::ResourceHandleType::BUFFER:
    {
        this->Bind(targetFrameIndex, set, binding, originalResource, originalResourceMeta.internal);
        break;
    }
    case ResourceHandle::ResourceHandleType::BUFFER_ARRAY:
    {
        auto bufferArray = static_cast<MutableBuffer *>(originalResource.get());
        bufferArray->ReSize(targetFrameIndex);
        this->Bind(targetFrameIndex, set, binding, bufferArray->GetBuffer(targetFrameIndex), originalResourceMeta.internal);
        break;
    }
    default:
        break;
    }
}
