cmake_minimum_required(VERSION 3.10)
project(char_test)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(char_test
        char_literals_dot_c_test.cpp)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory($ENV{UTBOT_ALL}/gtest
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)
file(GLOB ALL_TESTS "${PROJECT_SOURCE_DIR}/test/framework/*.cpp")

target_include_directories(char_test PUBLIC "${PROJECT_SOURCE_DIR}" "../" $ENV{UTBOT_ALL}/gtest/googletest)

target_link_libraries(
        char_test
        PUBLIC
        gtest_main
)