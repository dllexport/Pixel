#pragma once

#include <unordered_map>

#include <Core/IntrusivePtr.h>

#include <vk_mem_alloc.h>

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

private:
    friend class ContextBuilder;

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