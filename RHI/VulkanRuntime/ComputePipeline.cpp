#include <RHI/VulkanRuntime/ComputePipeline.h>

VulkanComputePipeline::VulkanComputePipeline(IntrusivePtr<Context> context, std::string pipelineName, std::string groupName) : Pipeline(pipelineName, groupName)
{
}

VulkanComputePipeline::~VulkanComputePipeline()
{
}

void VulkanComputePipeline::Build()
{
}