#pragma once

#include <cstdint>
#include <Core/IntrusivePtr.h>
#include <RHI/ResourceHandle.h>
#include <RHI/Memory.h>

class Buffer : public ResourceHandle
{
public:
    enum Type
    {
        BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000001,
        BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002,
        BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT = 0x00000004,
        BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT = 0x00000008,
        BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010,
        BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020,
        BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040,
        BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080,
        BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100,
        BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT = 0x00020000,

        BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT = 0x00000800,
        BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT = 0x00001000,
        BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT = 0x00000200,
        BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR = 0x00080000,
        BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR = 0x00100000,
        BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR = 0x00000400,

        BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT = 0x00200000,
        BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT = 0x00400000,
        BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT = 0x04000000,
        BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT = 0x00800000,
        BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT = 0x01000000,
        BUFFER_USAGE_RAY_TRACING_BIT_NV = BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
        BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_EXT = BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR = BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        BUFFER_USAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };

    using TypeBits = uint32_t;

    virtual void *Map() = 0;
    virtual size_t Size() = 0;

    // clone buffer with same settings
    virtual IntrusivePtr<Buffer> Clone() = 0;

protected:
    Buffer() : ResourceHandle(ResourceHandleType::BUFFER) {}
    virtual ~Buffer() = default;

    virtual bool Allocate(TypeBits type, MemoryPropertyBits memoryProperties, uint32_t size) = 0;
};