#pragma once

#include <string>
#include <fstream>

inline std::string ReadStringFile(std::string path)
{
    std::ifstream t(path);
    if (!t.is_open())
    {
        t.open("Shaders/" + path);
    }
    if (!t.is_open())
    {
        t.open("../Shaders/" + path);
    }
    return std::string((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}

inline std::vector<char> ReadBinaryFile(std::string path)
{
    std::ifstream is(path, std::ios::binary | std::ios::in | std::ios::ate);
    if (!is.is_open())
    {
        is.open("Shaders/" + path, std::ios::binary | std::ios::in | std::ios::ate);
    }
    if (!is.is_open())
    {
        is.open("../Shaders/" + path, std::ios::binary | std::ios::in | std::ios::ate);
    }
    if (is.is_open())
    {
        size_t size = is.tellg();
        is.seekg(0, std::ios::beg);
        std::vector<char> data(size);
        is.read(data.data(), size);
        is.close();
        return data;
    }
    else
    {
        return {};
    }
}