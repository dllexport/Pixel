#pragma once

#include <vector>

namespace Engine
{
    class Buffer
    {
    public:
        Buffer(void* data, uint32_t size) {
            bufferData.resize(size);
            memcpy(bufferData.data(), data, size);
        }
        
    private:
        std::vector<char> bufferData;
    };
};