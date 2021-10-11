cmake_minimum_required(VERSION 3.10)
project(char_test)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories("${PROJECT_SOURCE_DIR}" "../")

add_library(char_test
        char_literals_test.cpp)
