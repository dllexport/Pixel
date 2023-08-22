add_executable(Texture ./Texture/Texture.cpp)
   
target_link_libraries(Texture PRIVATE
    Pixel
)

add_executable(Triangle Triangle.cpp)
   
target_link_libraries(Triangle PRIVATE
    Pixel
)