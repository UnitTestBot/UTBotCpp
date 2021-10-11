cmake_minimum_required(VERSION 3.10)
project(regression)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-O2)

add_executable(
        regression
        helloworld.c
        SAT-752.c
        SAT-766.c
        SAT-767.c
        SAT-777.c
        main.c)

add_library(
        SAT-760
        SAT-760/SAT-760_1.c
        SAT-760/SAT-760_2.c
)

set_target_properties(regression PROPERTIES LINK_WHAT_YOU_USE TRUE)