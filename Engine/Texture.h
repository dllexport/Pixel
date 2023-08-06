#pragma once

#include <string>
#include <vector>

namespace Engine
{
    class Texture
    {
    public:
        Texture() = default;
        bool Load(std::string path);

    private:
        std::vector<char *> data;
    };
};
