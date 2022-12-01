cmake_minimum_required(VERSION 3.10)
project(error)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(error
        methods_with_asserts.cpp
        methods_with_exceptions.cpp)
