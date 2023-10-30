add_executable(ImGUI main.cpp)
   
target_link_libraries(ImGUI PRIVATE
    Pixel
)

configure_file("imgui.json" "imgui.json" COPYONLY)
configure_file("ui.vert.spv" "ui.vert.spv" COPYONLY)
configure_file("ui.frag.spv" "ui.frag.spv" COPYONLY)
