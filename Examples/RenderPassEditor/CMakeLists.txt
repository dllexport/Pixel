
file(GLOB IMGUI_EDITOR_SOURCE_FILES
    "../../3rd/imgui-node-editor/*.h"
    "../../3rd/imgui-node-editor/*.cpp"
)

add_executable(RenderPassEditor main.cpp ${IMGUI_EDITOR_SOURCE_FILES})

target_include_directories(RenderPassEditor PRIVATE "../../3rd/imgui-node-editor")

target_link_libraries(RenderPassEditor PRIVATE
    Pixel
)