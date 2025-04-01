cmake_minimum_required(VERSION 3.10)
project(my_raylib_project CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("C:/raylib/raylib/src")

add_executable(my_raylib_project main.cpp)
target_link_libraries(my_raylib_project "C:/raylib/raylib/src/libraylib.a" -lopengl32 -lgdi32 -lwinmm)
