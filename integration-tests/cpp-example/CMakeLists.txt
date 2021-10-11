cmake_minimum_required(VERSION 3.16)
project(cpp_project)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

# required!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_LINK_COMMANDS ON)

add_subdirectory(cxx_lib)
add_subdirectory(c_lib)
add_executable(cpp-proj SimpleClass.cpp Refs.cpp main.cpp)
