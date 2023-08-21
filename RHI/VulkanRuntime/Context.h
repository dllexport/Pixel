#pragma once

#include <unordered_map>

#include <RHI/VulkanRuntime/Instance.h>
#include <Core/IntrusivePtr.h>

#include <vk_mem_alloc.h>

class RHIRuntime;
class Context : public IntrusiveCounter<Context>
{
public:
    Context() = default;
    ~Context();

    struct DeviceQueue
    {
        VkQueue queue;
        uint32_t familyIndex;
    };

    VkInstance &GetVkInstance()
    {
        return instance;
    }

    VkPhysicalDevice &GetVkPhysicalDevice()
    {
        return physicalDevice;
    }

    VkDevice &GetVkDevice()
    {
        return logicalDevice;
    }

    VmaAllocator &GetVmaAllocator()
    {
        return vmaAllocator;
    }

    DeviceQueue GetQueue(VkQueueFlagBits queueType)
    {
        return queueContextMap[queueType];
    }

    const RHIRuntime *GetRHIRuntime()
    {
        return runtime;
    }

private:
    friend class ContextBuilder;
    friend class VulkanRuntime;

    RHIRuntime *runtime;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    void initInstance();

    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;

    std::unordered_map<VkQueueFlagBits, DeviceQueue> queueContextMap;

    VmaAllocator vmaAllocator;

    std::vector<VkExtensionProperties> enabledInstanceExtensions;
    std::vector<VkLayerProperties> enabledInstanceLayers;
};