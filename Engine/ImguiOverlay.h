#pragma once

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <Engine/PixelEngine.h>

#include <RHI/ResourceBindingState.h>
#include <RHI/Texture.h>
#include <RHI/MutableBuffer.h>
#include <RHI/RHIRuntime.h>

class ImGuiContext;
class ImguiOverlay : public IntrusiveCounter<ImguiOverlay>
{
public:
    ImguiOverlay(PixelEngine *engine);
    virtual ~ImguiOverlay();

    PixelEngine *engine;

    virtual void ImGUINewFrame();

    struct ImguiPushConstant
    {
        glm::vec2 scale;
        glm::vec2 translate;
    };

    IntrusivePtr<ResourceBindingState> drawState;
    IntrusivePtr<Pipeline> imguiPipeline;

    void BuildPipeline();

    // return update callback
    void BuildDrawable();

protected:
    ImGuiContext *imguiContext;
};