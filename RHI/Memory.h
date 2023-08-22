#pragma once

#include <cstdint>

namespace MemoryProperty
{
    enum : uint32_t
    {
        MEMORY_PROPERTY_DEVICE_LOCAL_BIT = 1 << 1,
        MEMORY_PROPERTY_HOST_VISIBLE_BIT = 1 << 2,
        MEMORY_PROPERTY_HOST_COHERENT_BIT = 1 << 3,
        MEMORY_PROPERTY_HOST_CACHED_BIT = 1 << 4,
        MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT = 1 << 5,
        MEMORY_PROPERTY_PROTECTED_BIT = 1 << 6,
        MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD = 1 << 7,
        MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD = 1 << 8,
        MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV = 1 << 9,

        // for constant buffer
        MEMORY_PROPERTY_HOST_LOCAL_BIT = 1 << 10,

        MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM = 1 << 11,
    };
};

using MemoryPropertyBits = uint32_t;
