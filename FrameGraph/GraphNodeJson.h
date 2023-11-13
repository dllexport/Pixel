#pragma once

#include <string>
#include <vector>

#include <json_struct/json_struct.h>

struct RenderSubPassResourceJson
{
    std::string name;
    std::string type;

    // attachment resourcefield
    std::string format;
    bool depthStencil;
    bool swapChain;
    bool shared;
    bool clear;

    uint8_t binding;
    uint8_t set;

    // for ssbo, ubo
    bool internal;
    bool immutable;
    uint32_t size;

    JS_OBJ(name, type, format, depthStencil, swapChain, shared, clear, set, binding, internal, immutable, size);
};

struct ShaderPaths
{
    std::string compute;
    std::string fragment;
    std::string vertex;
    JS_OBJ(compute, fragment, vertex);
};

struct RenderSubPassJson
{
    std::string name;
    std::string type;
    std::vector<std::string> dependencies;
    std::vector<RenderSubPassResourceJson> inputs;
    std::vector<RenderSubPassResourceJson> outputs;
    ShaderPaths shaders;
    JS_OBJ(name, type, dependencies, inputs, outputs, shaders);
};

struct RenderPassJson
{
    std::string name;
    std::vector<RenderSubPassJson> subpasses;

    JS_OBJ(name, subpasses);
};