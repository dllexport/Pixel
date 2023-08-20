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

enum ColorBlendFactor
{
    BLEND_FACTOR_ZERO = 0,
    BLEND_FACTOR_ONE = 1,
    BLEND_FACTOR_SRC_COLOR = 2,
    BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3,
    BLEND_FACTOR_DST_COLOR = 4,
    BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5,
    BLEND_FACTOR_SRC_ALPHA = 6,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7,
    BLEND_FACTOR_DST_ALPHA = 8,
    BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9,
    BLEND_FACTOR_CONSTANT_COLOR = 10,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 11,
    BLEND_FACTOR_CONSTANT_ALPHA = 12,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13,
    BLEND_FACTOR_SRC_ALPHA_SATURATE = 14,
    BLEND_FACTOR_SRC1_COLOR = 15,
    BLEND_FACTOR_ONE_MINUS_SRC1_COLOR = 16,
    BLEND_FACTOR_SRC1_ALPHA = 17,
    BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA = 18,
    BLEND_FACTOR_MAX_ENUM = 0x7FFFFFFF
};

enum ColorBlendOp
{
    BLEND_OP_ADD = 0,
    BLEND_OP_SUBTRACT = 1,
    BLEND_OP_REVERSE_SUBTRACT = 2,
    BLEND_OP_MIN = 3,
    BLEND_OP_MAX = 4,
};

enum ColorComponentFlagBits
{
    COLOR_COMPONENT_R_BIT = 0x00000001,
    COLOR_COMPONENT_G_BIT = 0x00000002,
    COLOR_COMPONENT_B_BIT = 0x00000004,
    COLOR_COMPONENT_A_BIT = 0x00000008,
    COLOR_COMPONENT_ALL_BIT = COLOR_COMPONENT_R_BIT | COLOR_COMPONENT_G_BIT | COLOR_COMPONENT_B_BIT | COLOR_COMPONENT_A_BIT
};
using ColorWriteMaskBits = uint32_t;

struct ColorBlendAttachmentState
{
    bool blendEnable;
    ColorBlendFactor srcColorBlendFactor;
    ColorBlendFactor dstColorBlendFactor;
    ColorBlendOp colorBlendOp;
    ColorBlendFactor srcAlphaBlendFactor;
    ColorBlendFactor dstAlphaBlendFactor;
    ColorBlendOp alphaBlendOp;
    ColorWriteMaskBits colorWriteMask;
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