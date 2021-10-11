cmake_minimum_required(VERSION 3.16)
project(module_project)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

# required!
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_LINK_COMMANDS ON)

add_subdirectory(lib/calc)
add_subdirectory(lib/literals)

# add_executable(module-project main.c)
# target_link_libraries(module-project PUBLIC calc literals)