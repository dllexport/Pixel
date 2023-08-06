#include <RHI/VulkanRuntime/ResourceBindingState.h>
#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/Buffer.h>

VulkanResourceBindingState::VulkanResourceBindingState(IntrusivePtr<Context> context, IntrusivePtr<Pipeline> pipeline) : ResourceBindingState(pipeline), context(context)
{
    AllocateDescriptorPool(pipeline);
    AllocateDescriptorSets();
}

VulkanResourceBindingState::~VulkanResourceBindingState()
{
    vkDestroyDescriptorPool(context->GetVkDevice(), descriptorPool, nullptr);
}

void VulkanResourceBindingState::Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource)
{
    WriteDescriptor(set, binding, resource);
}

void VulkanResourceBindingState::AllocateDescriptorPool(IntrusivePtr<Pipeline> pipeline)
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
            descriptorPoolSize.descriptorCount = binding.descriptorCount;
            poolSizes.push_back(descriptorPoolSize);
        }
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = uint32_t(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.maxSets = 1;

    auto result = vkCreateDescriptorPool(context->GetVkDevice(), &descriptorPoolInfo, nullptr, &descriptorPool);
}

void VulkanResourceBindingState::AllocateDescriptorSets()
{
    auto vulkanPipeline = static_cast<VulkanPipeline *>(pipeline.get());
    auto layouts = vulkanPipeline->GetPipelineLayout()->GetSetsLayouts();

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.pSetLayouts = layouts.data();
    descriptorSetAllocateInfo.descriptorSetCount = uint32_t(layouts.size());

    descriptorSets.resize(layouts.size());
    auto result = vkAllocateDescriptorSets(context->GetVkDevice(), &descriptorSetAllocateInfo, descriptorSets.data());
}

void VulkanResourceBindingState::WriteDescriptor(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource)
{
    // TODO, support different types of resource
    std::vector<VkWriteDescriptorSet> writeDescriptorSets;

    auto buffer = dynamic_cast<VulkanBuffer *>(resource.get());

    VkDescriptorBufferInfo bufferInfo = {
        .buffer = buffer->GetBuffer(),
        .offset = 0,
        .range = VK_WHOLE_SIZE};

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = descriptorSets[set];
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.dstBinding = binding;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.descriptorCount = 1;

    writeDescriptorSets.push_back(writeDescriptorSet);

    vkUpdateDescriptorSets(context->GetVkDevice(), (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}
