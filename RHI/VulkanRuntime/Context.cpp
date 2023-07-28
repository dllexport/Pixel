#include <RHI/VulkanRuntime/Context.h>

Context::~Context()
{
    vmaDestroyAllocator(vmaAllocator);
    vkDestroyDevice(logicalDevice, nullptr);
    physicalDevice = nullptr;

    if (debugMessenger)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        func(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);
}