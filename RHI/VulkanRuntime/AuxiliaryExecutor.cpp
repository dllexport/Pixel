#include <RHI/VulkanRuntime/AuxiliaryExecutor.h>
#include <RHI/VulkanRuntime/Runtime.h>
#include <RHI/VulkanRuntime/Buffer.h>

#include <vector>
#include <stdexcept>

VulkanAuxiliaryExecutor::VulkanAuxiliaryExecutor(IntrusivePtr<Context> context) : context(context)
{
    prepareCommandPool();
}

VulkanAuxiliaryExecutor::~VulkanAuxiliaryExecutor()
{
    vkDestroyCommandPool(context->GetVkDevice(), graphicCommandPool, nullptr);
    vkDestroyCommandPool(context->GetVkDevice(), computeCommandPool, nullptr);
}

bool VulkanAuxiliaryExecutor::Execute()
{
    std::vector<VkCommandBuffer> commandBuffers;
    if (submitGroups.empty())
    {
        return false;
    }
    for (auto &submitGroup : submitGroups)
    {
        commandBuffers.push_back(submitGroup.commandBuffer);
    }

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    vkCreateFence(context->GetVkDevice(), &fenceInfo, nullptr, &fence);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    submitInfo.pCommandBuffers = commandBuffers.data();

    if (vkQueueSubmit(context->GetQueue(VK_QUEUE_GRAPHICS_BIT).queue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit auxiliary command buffer!");
    }

    auto result = vkWaitForFences(context->GetVkDevice(), 1, &fence, VK_TRUE, UINT32_MAX);
    vkDestroyFence(context->GetVkDevice(), fence, nullptr);
    submitGroups.clear();
    return true;
}

void VulkanAuxiliaryExecutor::WaitIdle()
{
    vkDeviceWaitIdle(context->GetVkDevice());
}

static void setImageLayout(
    VkCommandBuffer cmdbuffer,
    VkImage image,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkImageSubresourceRange subresourceRange,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask)
{
    // Create an image barrier object
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        imageMemoryBarrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newImageLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (imageMemoryBarrier.srcAccessMask == 0)
        {
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier(
        cmdbuffer,
        srcStageMask,
        dstStageMask,
        0,
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier);
}

void VulkanAuxiliaryExecutor::TransferResource(IntrusivePtr<Texture> gpuTexture, IntrusivePtr<Buffer> hostBuffer)
{
    auto texture = static_cast<VulkanTexture *>(gpuTexture.get());
    auto stagingBuffer = static_cast<VulkanBuffer *>(hostBuffer.get());

    auto commandBuffer = allocateCommandBuffer(graphicCommandPool);
    VkCommandBufferBeginInfo cmdBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    setImageLayout(commandBuffer,
                   texture->GetImage(),
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   subresourceRange,
                   VK_PIPELINE_STAGE_HOST_BIT,
                   VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = texture->GetExtent().width;
    bufferCopyRegion.imageExtent.height = texture->GetExtent().height;
    bufferCopyRegion.imageExtent.depth = texture->GetExtent().depth;

    vkCmdCopyBufferToImage(
        commandBuffer,
        stagingBuffer->GetBuffer(),
        texture->GetImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &bufferCopyRegion);

    setImageLayout(commandBuffer,
                   texture->GetImage(),
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   subresourceRange,
                   VK_PIPELINE_STAGE_TRANSFER_BIT,
                   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

    vkEndCommandBuffer(commandBuffer);

    this->submitGroups.push_back(SubmitGroup{
        .commandBuffer = commandBuffer,
        .texture = texture,
        .buffer = stagingBuffer});
}

void VulkanAuxiliaryExecutor::TransferResource(IntrusivePtr<Buffer> gpuBuffer, IntrusivePtr<Buffer> hostBuffer)
{
}

void VulkanAuxiliaryExecutor::prepareCommandPool()
{
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = context->GetQueue(VK_QUEUE_GRAPHICS_BIT).familyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        auto result = vkCreateCommandPool(context->GetVkDevice(), &cmdPoolInfo, nullptr, &graphicCommandPool);
    }

    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = context->GetQueue(VK_QUEUE_COMPUTE_BIT).familyIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        auto result = vkCreateCommandPool(context->GetVkDevice(), &cmdPoolInfo, nullptr, &computeCommandPool);
    }
}

VkCommandBuffer VulkanAuxiliaryExecutor::allocateCommandBuffer(VkCommandPool pool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = pool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(context->GetVkDevice(), &commandBufferAllocateInfo, &cmdBuffer);
    return cmdBuffer;
}
