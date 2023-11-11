#pragma once

#include <memory>
#include <unordered_set>

#include <glm/glm.hpp>

#include <Core/IntrusivePtr.h>
#include <RHI/Pipeline.h>
#include <RHI/ResourceHandle.h>

#include <Engine/Event.h>

// state descriping one pipeline
class ResourceBindingState : public IntrusiveCounter<ResourceBindingState>
{
public:
    // for debug
    std::string name;

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
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        int32_t vertexOffset;
        uint32_t firstInstance;
    };

    struct DispatchOP
    {
        uint32_t x, y, z;
    };

    void BindDrawOp(std::vector<DrawOP> drawOps)
    {
        this->drawOps = drawOps;
    }

    std::vector<DrawOP> &GetDrawOps()
    {
        return this->drawOps;
    }

    void BindDispatchOp(std::vector<DispatchOP> dispatchOps)
    {
        this->dispatchOps = dispatchOps;
    }

    std::vector<DispatchOP> &GetDispatchOps()
    {
        return this->dispatchOps;
    }

    enum IndexType
    {
        INDEX_TYPE_UINT16 = 0,
        INDEX_TYPE_UINT32 = 1,
        INDEX_TYPE_UINT8 = 1000265000,
    };

    void BindVertexBuffer(IntrusivePtr<ResourceHandle> buffer)
    {
        vertexBuffer = buffer;
    }

    void BindIndexBuffer(IntrusivePtr<ResourceHandle> buffer, IndexType type)
    {
        indexBuffer = buffer;
        indexType = type;
    }

    IntrusivePtr<ResourceHandle> &GetVertexBuffers()
    {
        return vertexBuffer;
    }

    IntrusivePtr<ResourceHandle> &GetIndexBuffers()
    {
        return indexBuffer;
    }

    void RegisterUpdateCallback(UpdateCallback callback)
    {
        updateCallbacks.push_back(callback);
    }

    void ClearUpdateCallbacks()
    {
        updateCallbacks.clear();
    }

protected:
    friend class std::hash<ResourceBindingState>;
    friend class Renderer;

    IntrusivePtr<Pipeline> pipeline;
    IntrusivePtr<ResourceHandle> vertexBuffer;
    IntrusivePtr<ResourceHandle> indexBuffer;
    IndexType indexType;

    // define how renderer will draw the buffer
    std::vector<DrawOP> drawOps;
    std::vector<DispatchOP> dispatchOps;

    std::vector<UpdateCallback> updateCallbacks;
};