#pragma once

#include <memory>
#include <unordered_set>

#include <glm/glm.hpp>

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

    virtual void Bind(IntrusivePtr<ResourceHandle> resource) = 0;
    virtual void Bind(uint32_t set, uint32_t binding, IntrusivePtr<ResourceHandle> resource) = 0;
    virtual void Bind(uint32_t set, uint32_t binding, std::vector<IntrusivePtr<ResourceHandle>> resources) = 0;

    // primitive draw type denpends on the pipeline setting
    struct DrawOP
    {
        glm::i32vec2 scissorOffset;
        glm::u32vec2 scissorExtent;

        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t vertexOffset;
        uint32_t firstInstance;
    };
    void BindDrawOp(std::vector<DrawOP> drawOps)
    {
        this->drawOps = drawOps;
    }

    std::vector<DrawOP> &GetDrawOps()
    {
        return this->drawOps;
    }

    enum IndexType
    {
        INDEX_TYPE_UINT16 = 0,
        INDEX_TYPE_UINT32 = 1,
        INDEX_TYPE_UINT8 = 1000265000,
    };

    void BindVertexBuffer(IntrusivePtr<Buffer> buffer)
    {
        vertexBuffer = buffer;
    }

    void BindIndexBuffer(IntrusivePtr<Buffer> buffer, IndexType type)
    {
        indexBuffer = buffer;
        indexType = type;
    }

    IntrusivePtr<Buffer> &GetVertexBuffer()
    {
        return vertexBuffer;
    }

    IntrusivePtr<Buffer> &GetIndexBuffer()
    {
        return indexBuffer;
    }

protected:
    friend class std::hash<ResourceBindingState>;
    friend class DrawableBinder;
    IntrusivePtr<Pipeline> pipeline;
    IntrusivePtr<Buffer> vertexBuffer;
    IntrusivePtr<Buffer> indexBuffer;
    IndexType indexType;

    // define how renderer will draw the buffer
    std::vector<DrawOP> drawOps;
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
