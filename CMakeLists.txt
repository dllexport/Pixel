cmake_minimum_required(VERSION 3.24)
project(Pixel)

set(CMAKE_CXX_STANDARD 20)

set(VULKAN_SDK "C:/VulkanSDK/1.3.239.0")
set(GLSL_VALIDATOR "${VULKAN_SDK}/Bin/glslangValidator")
     
add_executable(Pixel main.cpp)
   
target_include_directories(Pixel PRIVATE
    ${VULKAN_SDK}/Include
)

target_link_libraries(Pixel PRIVATE
    ${VULKAN_SDK}/Lib/vulkan-1.lib
)