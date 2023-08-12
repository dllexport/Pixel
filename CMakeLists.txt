cmake_minimum_required(VERSION 3.24)
project(Pixel)

set(CMAKE_CXX_STANDARD 20)

set(VULKAN_SDK "C:/VulkanSDK/1.3.239.0")
set(BOOST_ROOT "C:/Users/Mario/Documents/boost_1_81_0/stage")
set(GLSL_VALIDATOR "${VULKAN_SDK}/Bin/glslangValidator")
 
if(MSVC)
  add_definitions(/MP)
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(3rd/VulkanMemoryAllocator/include)

find_package(Boost COMPONENTS date_time filesystem system)

add_subdirectory(3rd/json_struct)
set(SPIRV_REFLECT_STATIC_LIB ON)
set(SPIRV_REFLECT_EXAMPLES OFF)
add_subdirectory(3rd/SPIRV-Reflect)
add_subdirectory(3rd/spdlog)

file(GLOB PIXEL_SOURCE_FILES
    "Core/*.*"
    "Engine/*.*"
    "FrameGraph/*.*"
    "RHI/*.*"
    "RHI/VulkanRuntime/*.*"
)

add_executable(Pixel main.cpp ${PIXEL_SOURCE_FILES})
   
target_include_directories(Pixel PRIVATE
    ${VULKAN_SDK}/Include
)

target_link_libraries(Pixel PRIVATE
    ${Boost_LIBRARIES}
    ${VULKAN_SDK}/Lib/vulkan-1.lib
    json_struct
    spdlog
    spirv-reflect-static
)

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