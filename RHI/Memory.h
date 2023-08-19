#pragma once

#include <cstdint>

namespace MemoryProperty
{
    enum : uint32_t
    {
        MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 0x00000001,
        MEMORY_PROPERTY_HOST_VISIBLE_BIT = 0x00000002,
        MEMORY_PROPERTY_HOST_COHERENT_BIT = 0x00000004,
        MEMORY_PROPERTY_HOST_CACHED_BIT = 0x00000008,
        MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = 0x00000010,
        MEMORY_PROPERTY_PROTECTED_BIT = 0x00000020,
        MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD = 0x00000040,
        MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD = 0x00000080,
        MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV = 0x00000100,

        // for constant buffer
        MEMORY_PROPERTY_HOST_LOCAL_BIT,

        MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
};

using MemoryPropertyBits = uint32_t;
