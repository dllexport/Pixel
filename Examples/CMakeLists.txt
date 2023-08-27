add_subdirectory(Texture)
add_subdirectory(Deferred)

add_executable(Triangle Triangle.cpp)
   
target_link_libraries(Triangle PRIVATE
    Pixel
)