cmake_minimum_required(VERSION 3.0)

project(Arena_Proj)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(SDL)
add_subdirectory(assimp)

add_executable(arena
    src/glad.c
    src/stb_image.c
    src/model.cpp
    src/main.cpp)
target_include_directories(arena PUBLIC SDL/include assimp/include src)
target_link_libraries(arena SDL2 assimp)
