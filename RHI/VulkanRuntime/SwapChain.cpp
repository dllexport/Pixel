#include <RHI/VulkanRuntime/SwapChain.h>

#include <stdexcept>

VulkanSwapChain::VulkanSwapChain(IntrusivePtr<Context> context) : context(context)
{
}

VulkanSwapChain::~VulkanSwapChain()
{
    for (int i = 0; i < imageAvailableSemaphores.size(); i++)
    {
        vkDestroySemaphore(context->GetVkDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(context->GetVkDevice(), renderFinishedSemaphores[i], nullptr);
    }

    vkDestroySwapchainKHR(context->GetVkDevice(), swapChain, nullptr);
}

uint32_t VulkanSwapChain::Acquire(uint32_t currentFrame)
{
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context->GetVkDevice(), swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return -1;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    return imageIndex;
}

bool VulkanSwapChain::Present(uint32_t index, uint32_t currentFrame)
{
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &index;

    auto result = vkQueuePresentKHR(context->GetQueue(VK_QUEUE_GRAPHICS_BIT).queue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return false;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    return true;
}

void VulkanSwapChain::InitSync(uint32_t imageSize)
{
    imageAvailableSemaphores.resize(imageSize);
    renderFinishedSemaphores.resize(imageSize);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < imageSize; i++)
    {
        if (vkCreateSemaphore(context->GetVkDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->GetVkDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}
