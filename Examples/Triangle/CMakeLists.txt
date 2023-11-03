add_executable(Triangle main.cpp)
   
target_link_libraries(Triangle PRIVATE
    Pixel
)

configure_file("triangle.json" "triangle.json" COPYONLY)
