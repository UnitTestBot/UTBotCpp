cmake_minimum_required(VERSION 3.10)
project(cli)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(cli
        inner/inner_basic_functions.c
        assertion_failures.c
        basic_functions.c
        complex_structs.c)
