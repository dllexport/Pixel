cmake_minimum_required(VERSION 3.20)
project(Pixel)

set(CMAKE_CXX_STANDARD 20)

set(VULKAN_SDK)
set(BOOST_ROOT)
set(GLSL_VALIDATOR "${VULKAN_SDK}/Bin/glslangValidator")

set(WINDOW_USE_GLFW ON)

if(MSVC)
    add_definitions(/MP)
endif()

include_directories(3rd/imgui)

file(GLOB IMGUI_SOURCE_FILES
    "3rd/imgui/*.h"
    "3rd/imgui/*.cpp"
)

find_package(Boost COMPONENTS date_time filesystem system)

set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_EXAMPLES OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)
add_subdirectory(3rd/SPIRV-Reflect)

add_subdirectory(3rd/json_struct)
add_subdirectory(3rd/spdlog)
add_subdirectory(3rd/glm)

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF)
set(TINYGLTF_INSTALL OFF)
add_subdirectory(3rd/tinygltf)

set(KTX_FEATURE_TOOLS OFF)
set(KTX_FEATURE_TESTS OFF)
add_subdirectory(3rd/KTX-Software)

if (WINDOW_USE_GLFW)
    add_definitions(-DWINDOW_USE_GLFW)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_INSTALL OFF)
    add_subdirectory(3rd/glfw)
    if (UNIX)
        set(GLFW_BUILD_X11 OFF)
        set(GLFW_USE_WAYLAND ON)
    endif (UNIX)
endif()

file(GLOB PIXEL_SOURCE_FILES
    "Core/*.*"
    "IO/*.*"
    "Engine/*.*"
    "FrameGraph/*.*"
    "RHI/*.*"
    "RHI/VulkanRuntime/*.*"
)

add_library(Pixel STATIC ${PIXEL_SOURCE_FILES} ${IMGUI_SOURCE_FILES})

target_include_directories(Pixel PRIVATE
    3rd/VulkanMemoryAllocator/include
    ${VULKAN_SDK}/include
)

target_include_directories(Pixel PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(Pixel PUBLIC
    ${Boost_LIBRARIES}
    json_struct
    spdlog
    glm
    spirv-reflect-static
    ktx
    tinygltf
)

if (WIN32)
    target_link_libraries(Pixel PUBLIC
        ${VULKAN_SDK}/lib/vulkan-1.lib
    ) 
elseif (APPLE)
    target_link_libraries(Pixel PUBLIC
        ${VULKAN_SDK}/lib/libvulkan.1.dylib
    )
endif()

if (WINDOW_USE_GLFW)
target_link_libraries(Pixel PRIVATE
    glfw
)
endif()

# add_subdirectory(3rd/googletest)
# enable_testing()
# add_executable(
#   framegraph_test
#   framegraph_test.cpp
# )

# target_link_libraries(
#   framegraph_test
#   GTest::gtest_main
# )

# include(GoogleTest)
# gtest_discover_tests(framegraph_test)

add_subdirectory(Examples)