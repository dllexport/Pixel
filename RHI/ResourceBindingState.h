#pragma once

#include <unordered_set>

#include <Core/IntrusivePtr.h>
#include <RHI/Pipeline.h>
#include <RHI/ResourceHandle.h>
#include <RHI/Buffer.h>

// state descriping one pipeline
class ResourceBindingState : public IntrusiveCounter<ResourceBindingState>
{
public:
    ResourceBindingState(IntrusivePtr<Pipeline> pipeline) : pipeline(pipeline) {}
    virtual ~ResourceBindingState() {}

    IntrusivePtr<Pipeline> &GetPipeline()
    {
        return pipeline;
    }

    virtual void Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource) = 0;
    virtual void Bind(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources) = 0;

    void BindVertexBuffer(IntrusivePtr<Buffer> buffer)
    {
        vertexBuffer = buffer;
    }

    void BindIndexBuffer(IntrusivePtr<Buffer> buffer)
    {
        indexBuffer = buffer;
    }

protected:
    friend class std::hash<ResourceBindingState>;
    friend class DrawableBinder;
    IntrusivePtr<Pipeline> pipeline;
    IntrusivePtr<Buffer> vertexBuffer;
    IntrusivePtr<Buffer> indexBuffer;
};

template <>
struct std::hash<ResourceBindingState>
{
    std::size_t operator()(const ResourceBindingState &k) const
    {
        // TODO HASH states
        return size_t(k.pipeline.get());
    }
};
