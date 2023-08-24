#pragma once

#include <string>

#include <Core/IntrusivePtr.h>

#include <Engine/PixelEngine.h>

struct GLTFVertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 joint0;
    glm::vec4 weight0;
    glm::vec4 tangent;
};

struct GLTFModelPrimitive
{
    uint32_t firstVertex;
    uint32_t vertexCount;

    uint32_t firstIndex;
    uint32_t indexCount;
};

struct GLTFModel
{
    std::vector<GLTFVertex> vertexBuffer;
    std::vector<uint32_t> indexBuffer;
    std::vector<GLTFModelPrimitive> primitives;
};

class GLTFReader
{
public:
    static std::vector<GLTFModel> ReadFile(PixelEngine *engine, std::string file);
};