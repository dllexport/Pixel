#pragma once

#include <RHI/VulkanRuntime/Context.h>

class ResourceUpdater
{
public:
    ResourceUpdater(IntrusivePtr<Context> context);
    ~ResourceUpdater();

private:
    IntrusivePtr<Context> context;
};
