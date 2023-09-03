#include <RHI/VulkanRuntime/ResourceBindingState.h>

#include <stdexcept>

#include <RHI/VulkanRuntime/Pipeline.h>
#include <RHI/VulkanRuntime/PipelineLayout.h>
#include <RHI/VulkanRuntime/Buffer.h>
#include <RHI/MutableBuffer.h>
#include <RHI/VulkanRuntime/Texture.h>
#include <RHI/VulkanRuntime/TextureView.h>
#include <RHI/VulkanRuntime/Sampler.h>

VulkanResourceBindingState::VulkanResourceBindingState(IntrusivePtr<Context> context, IntrusivePtr<Pipeline> pipeline) : ResourceBindingState(pipeline), context(context)
{
    this->descriptorSet = new VulkanDescriptorSet(context, pipeline);
}

VulkanResourceBindingState::~VulkanResourceBindingState()
{
}

void VulkanResourceBindingState::Bind(IntrusivePtr<ResourceHandle> resource)
{
    descriptorSet->Bind(resource);
}

void VulkanResourceBindingState::Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource)
{
    descriptorSet->Bind(set, binding, resource);
}

void VulkanResourceBindingState::Bind(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources)
{
    descriptorSet->Bind(set, binding, resources);
}

void VulkanResourceBindingState::BindInternal(uint32_t frameIndex, uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource)
{
    descriptorSet->BindInternal(frameIndex, set, binding, resource);
}

void VulkanResourceBindingState::Copy(uint32_t targetFrameIndex, uint32_t set, uint32_t binding)
{
    descriptorSet->Copy(targetFrameIndex, set, binding);
}
