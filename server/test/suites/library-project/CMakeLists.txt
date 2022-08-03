cmake_minimum_required(VERSION 3.16)
project(library-project)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(CMAKE_EXPORT_LINK_COMMANDS ON)

add_compile_options(-O2)

add_subdirectory(lib)
add_subdirectory(src)
