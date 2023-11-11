add_executable(ImGUI main.cpp)
   
target_link_libraries(ImGUI PRIVATE
    Pixel
)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "Shaders/*.frag"
    "Shaders/*.vert"
)

APPEND_GLSL_TO_TARGET(ImGUI "${GLSL_SOURCE_FILES}")

configure_file("Shaders/imgui.json" "Shaders/imgui.json")
