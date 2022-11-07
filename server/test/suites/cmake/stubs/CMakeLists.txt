cmake_minimum_required(VERSION 3.13)

project(cmake)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(exe main.c)
target_include_directories(exe PUBLIC unitB)
add_subdirectory(unitB)
add_subdirectory(unitA)
target_link_libraries(exe unitA unitB)
