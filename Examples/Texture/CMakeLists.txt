add_executable(Texture main.cpp)
   
target_link_libraries(Texture PRIVATE
    Pixel
)

configure_file("texture.json" "texture.json" COPYONLY)
