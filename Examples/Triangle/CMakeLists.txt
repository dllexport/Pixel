add_executable(Triangle main.cpp)
   
target_link_libraries(Triangle PRIVATE
    Pixel
)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "Shaders/*.frag"
    "Shaders/*.vert"
    "../ImGUI/Shaders/ui.vert"
    "../ImGUI/Shaders/ui.frag"
)

APPEND_GLSL_TO_TARGET(Triangle "${GLSL_SOURCE_FILES}")

configure_file("Shaders/triangle.json" "Shaders/triangle.json")
configure_file("Shaders/imgui.json" "Shaders/imgui.json")

