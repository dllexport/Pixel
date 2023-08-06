#include <RHI/VulkanRuntime/PipelineLayout.h>

#include <spirv_reflect.h>

#include <spdlog/spdlog.h>

#include <RHI/VulkanRuntime/SPIVReflection.h>

VulkanPipelineLayout::VulkanPipelineLayout(IntrusivePtr<Context> context) : context(context)
{
}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
    auto device = context->GetVkDevice();
    for (auto &desriptorSetLayout : desriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(device, desriptorSetLayout, nullptr);
    }

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}

void VulkanPipelineLayout::Build(std::vector<IntrusivePtr<SPIVReflection>> reflections)
{
    bindingsInSets.resize(8);
    for (auto reflection : reflections)
    {
        auto descriptorState = reflection->ParseDescriptorLayoutState();
        // for each set = i
        for (int i = 0; i < descriptorState.descriptorSetLayoutSets.size(); i++)
        {
            auto layoutInSet = descriptorState.descriptorSetLayoutSets[i];
            std::copy(layoutInSet.begin(), layoutInSet.end(), std::back_inserter(bindingsInSets[i]));
        }
    }

    for (auto bindingsInSet : bindingsInSets)
    {
        if (bindingsInSet.empty())
        {
            break;
        }
        VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
        descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayout.pNext = nullptr;
        descriptorLayout.bindingCount = (uint32_t)bindingsInSet.size();
        descriptorLayout.pBindings = bindingsInSet.data();
        VkDescriptorSetLayout layout;
        auto result = vkCreateDescriptorSetLayout(context->GetVkDevice(), &descriptorLayout, nullptr, &layout);
        desriptorSetLayouts.push_back(layout);
    }

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = desriptorSetLayouts.size();
    pPipelineLayoutCreateInfo.pSetLayouts = desriptorSetLayouts.data();

    auto result = vkCreatePipelineLayout(context->GetVkDevice(), &pPipelineLayoutCreateInfo, nullptr, &this->pipelineLayout);
}

void VulkanPipelineLayout::ParseFromReflect(std::vector<char> spirv_code)
{
}
