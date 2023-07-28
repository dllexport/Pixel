//
// Created by Mario Lau on 2023/1/18.
//
#pragma once

#include <vector>

#include <RHI/VulkanRuntime/Context.h>

class ContextBuilder {
public:
    ContextBuilder();

    boost::intrusive_ptr<Context> Build();
    boost::intrusive_ptr<Context> DefaultBuild();
    ContextBuilder& SetInstanceExtensions(std::vector<const char*>&& instanceExtensions);
    ContextBuilder& SetInstanceLayers(std::vector<const char*>&& instanceLayers);
    ContextBuilder& SetDeviceExtensions(std::vector<const char*>&& extensions);
    ContextBuilder& SetDeviceLayers(std::vector<const char*>&& layers);
    ContextBuilder& EnableValidationLayer();
    ContextBuilder& BuildDebugUtilsMessenger(PFN_vkDebugUtilsMessengerCallbackEXT callback);
    ContextBuilder& SelectPhysicalDevice(std::function<int(std::vector<VkPhysicalDevice>)> selector);

private:
    boost::intrusive_ptr<Context> context;
    std::vector<const char*> instanceExtensions;
    std::vector<const char*> instanceLayers;
    std::vector<const char*> deviceExtensions;
    std::vector<const char*> deviceLayers;

    std::vector<VkExtensionProperties> availableInstanceExtensions;
    std::vector<VkLayerProperties> availableInstanceLayers;
    std::vector<VkExtensionProperties> availablePhysicalDeviceExtensions;
    std::vector<VkLayerProperties> availablePhysicalDeviceLayers;

    bool enableValidationLayers;
    PFN_vkDebugUtilsMessengerCallbackEXT debugUtilsMessengerCallback;

    // pick discrete GPU by default
    static int DefaultPhysicalDeviceSelector(std::vector<VkPhysicalDevice> devices);
    std::function<int(std::vector<VkPhysicalDevice>)> physicalDeviceSelector = DefaultPhysicalDeviceSelector;

    void BuildInstance();
    void BuildPhysicalDevice();
    void BuildLogicalDevice();
    void BuildVMA();
};


