cmake_minimum_required(VERSION 3.20)
project(Pixel)

set(CMAKE_CXX_STANDARD 20)

set(VULKAN_SDK "C:/VulkanSDK/1.3.239.0")
set(BOOST_ROOT "C:/Users/Mario/Documents/boost_1_81_0/stage")
set(GLSL_VALIDATOR "${VULKAN_SDK}/Bin/glslangValidator")

set(WINDOW_USE_GLFW ON)

if(MSVC)
    add_definitions(/MP)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(3rd/VulkanMemoryAllocator/include)
include_directories(3rd/imgui)

file(GLOB IMGUI_SOURCE_FILES
    "3rd/imgui/*.cpp"
)

find_package(Boost COMPONENTS date_time filesystem system)

set(SPIRV_REFLECT_EXECUTABLE OFF)
set(SPIRV_REFLECT_EXAMPLES OFF)
set(SPIRV_REFLECT_STATIC_LIB ON)
set(SPIRV_REFLECT_EXAMPLES OFF)
add_subdirectory(3rd/SPIRV-Reflect)

add_subdirectory(3rd/json_struct)
add_subdirectory(3rd/spdlog)

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

add_executable(Pixel main.cpp ${PIXEL_SOURCE_FILES} ${IMGUI_SOURCE_FILES})
   
target_include_directories(Pixel PRIVATE
    ${VULKAN_SDK}/include
)

target_link_libraries(Pixel PRIVATE
    ${Boost_LIBRARIES}
    ${VULKAN_SDK}/lib/vulkan-1.lib
    json_struct
    spdlog
    spirv-reflect-static
    ktx
)

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