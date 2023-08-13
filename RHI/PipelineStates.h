#pragma once

#include <vector>
#include <string>

struct InputAssembleState
{
    enum class Type
    {
        POINT,
        LINE_LIST,
        LINE_STRIP,
        TRIANGLE_LIST,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
    };

    Type type;
};

struct RasterizationState
{
    enum class FrontFaceType
    {
        COUNTER_CLOCKWISE,
        CLOCKWISE
    };

    enum class CullModeType
    {
        NONE,
        FRONT,
        BACK,
        FRONT_AND_BACK
    };

    enum class PolygonModeType
    {
        FILL,
        LINE,
        POINT,
    };

    PolygonModeType polygonMode;
    CullModeType cullMode;
    FrontFaceType frontFace;
    float lineWidth = 1;
};

struct ColorBlendAttachmentState
{
    bool blendEnable;
    bool colorWriteMask;
};

struct DepthStencilState
{
    bool depthTestEnable;
    bool depthWriteEnable;
};

struct ShaderState
{
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::string commputeShaderPath;

    bool Empty()
    {
        return vertexShaderPath.empty() || fragmentShaderPath.empty();
    }
};

// aggregate states
struct PipelineStates
{
    InputAssembleState inputAssembleState;
    RasterizationState rasterizationState;
    std::vector<ColorBlendAttachmentState> colorBlendAttachmentStates;
    DepthStencilState depthStencilState;
    ShaderState shaderState;
};