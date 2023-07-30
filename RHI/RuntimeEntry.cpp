#include <RHI/RuntimeEntry.h>

#include <RHI/VulkanRuntime/Runtime.h>

IntrusivePtr<RHIRuntime> RuntimeEntry::Create()
{
    switch (type)
    {
    case Type::VULKAN:
    {
        return new VulkanRuntime();
    }
    default:
        return nullptr;
    }
}
