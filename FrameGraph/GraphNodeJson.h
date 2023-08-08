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

    JS_OBJ(name, type, format, depthStencil, swapChain, shared);
};

struct RenderSubPassJson
{
    std::string name;
    std::string type;
    std::vector<std::string> subpass_dependency;
    std::vector<RenderSubPassResourceJson> inputs;
    std::vector<RenderSubPassResourceJson> outputs;

    JS_OBJ(name, type, subpass_dependency, inputs, outputs);
};

struct RenderPassJson
{
    std::string name;
    std::vector<RenderSubPassJson> subpasses;

    JS_OBJ(name, subpasses);
};