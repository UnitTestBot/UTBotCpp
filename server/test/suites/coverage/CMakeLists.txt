cmake_minimum_required(VERSION 3.10)
project(coverage1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(coverage1
        basic_functions.c
        simple_loop_uncovered.c
        dependent_functions.c
        simple_class.cpp
        methods_with_exceptions.cpp
        methods_with_asserts.cpp)
