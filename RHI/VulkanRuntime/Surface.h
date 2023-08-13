#pragma once

#include <Core/IntrusivePtr.h>

#include <RHI/VulkanRuntime/Context.h>

#include <vulkan/vulkan.h>

class Surface : public IntrusiveCounter<Surface>
{
public:
    Surface(IntrusivePtr<Context> context, void *handle);
    ~Surface();
    
    void Build();

    VkSurfaceKHR GetSurface();

private:
    IntrusivePtr<Context> context;
    void *handle;
    VkSurfaceKHR surface;
};