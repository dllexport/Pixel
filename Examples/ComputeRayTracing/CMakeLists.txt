add_executable(ComputeRayTracing main.cpp)
   
target_link_libraries(ComputeRayTracing PRIVATE
    Pixel
)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "Shaders/*.frag"
    "Shaders/*.vert"
    "Shaders/*.comp"
    "../ImGUI/Shaders/ui.vert"
    "../ImGUI/Shaders/ui.frag"
)

APPEND_GLSL_TO_TARGET(ComputeRayTracing "${GLSL_SOURCE_FILES}")

configure_file("Shaders/imgui.json" "Shaders/imgui.json")
configure_file("Shaders/ComputeRayTracing.json" "Shaders/ComputeRayTracing.json")
