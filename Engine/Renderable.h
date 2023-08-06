#pragma once

#include <string>

#include <Core/IntrusivePtr.h>

#include <RHI/Texture.h>
#include <RHI/Buffer.h>
#include <RHI/ResourceBindingState.h>

namespace Engine
{
    class Renderable : public IntrusiveCounter<Renderable>
    {
    public:
        void SetVertexBuffer(IntrusivePtr<Buffer> buffer) { vertexBuffer = buffer; }
        void SetInstanceBuffer(IntrusivePtr<Buffer> buffer) { instanceBuffer = buffer; }
        void SetIndexBuffer(IntrusivePtr<Buffer> buffer) { indexBuffer = buffer; }

    private:
        IntrusivePtr<ResourceHandle> vertexBuffer;
        IntrusivePtr<ResourceHandle> indexBuffer;
        IntrusivePtr<ResourceHandle> instanceBuffer;
    };
};