#pragma once

#include <RHI/Executor.h>
#include <RHI/Texture.h>
#include <RHI/Buffer.h>

class AuxiliaryExecutor : public Executor
{
public:
    virtual ~AuxiliaryExecutor() = default;
    
    virtual void TransferResource(IntrusivePtr<Texture> gpuTexture, IntrusivePtr<Buffer> hostBuffer) = 0;
    virtual void TransferResource(IntrusivePtr<Buffer> gpuBuffer, IntrusivePtr<Buffer> hostBuffer) = 0;
};