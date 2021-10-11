cmake_minimum_required(VERSION 3.10)
project(datacom)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(one a.c)
add_library(two b.c)

add_executable(ex main.c)
target_link_libraries(ex one)
target_link_libraries(ex two)
