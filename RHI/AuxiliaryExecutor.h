#pragma once

#include <RHI/Executor.h>
#include <RHI/Texture.h>
#include <RHI/Buffer.h>

class AuxiliaryExecutor : public Executor
{
public:
    virtual ~AuxiliaryExecutor() = default;
    
    struct TransferConfig
    {
        std::vector<uint64_t> mipmapBufferLevelOffsets;
    };

    // clear all idle states
    virtual void Reset() = 0;

    virtual void TransferResource(IntrusivePtr<Texture> gpuTexture, IntrusivePtr<Buffer> hostBuffer, TransferConfig config = {}) = 0;
    virtual void TransferResource(IntrusivePtr<Buffer> gpuBuffer, IntrusivePtr<Buffer> hostBuffer) = 0;
};