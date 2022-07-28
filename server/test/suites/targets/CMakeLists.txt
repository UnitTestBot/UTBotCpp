cmake_minimum_required(VERSION 3.10)
project(targets)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-O2)

add_library(parse STATIC parse.c)
add_executable(ls ls.c)
add_executable(cat cat.c)
add_executable(dummy dummy.c)

add_executable(get_10 get_10.c shared.c get_val_main.c)
add_executable(get_20 get_20x.c shared.c get_val_main.c)
add_executable(get_10_2 get_10.c shared.c get_val_main_2.c)

add_library(shared_get SHARED get_20x.c shared.c get_val_main.c)

add_library(static_get STATIC get_20x.c shared.c get_val_main.c)
target_compile_definitions(static_get PRIVATE TESTDEF)

target_link_libraries(ls parse)
target_link_libraries(cat parse)
target_link_libraries(dummy parse)
