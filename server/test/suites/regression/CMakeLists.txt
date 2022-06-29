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

add_library(
        PR-200
        PR-200/pair.c
        PR-200/PR-200.c
)

add_library(PR120 PR120.c)
set_property(TARGET PR120 PROPERTY C_STANDARD 90)

add_library(PR123 PR123.c)

add_library(PR124 PR124.c)

add_library(PR153 PR153.c)

add_library(GH215 GH215.c)

add_library(issue-276 issue-276.c)
target_compile_definitions(issue-276 PUBLIC EXPORT1=)
target_compile_definitions(issue-276 PUBLIC EXPORT2="")
target_compile_definitions(issue-276 PUBLIC EXPORT3=4)
target_compile_definitions(issue-276 PUBLIC EXPORT4="4")

set_target_properties(regression PROPERTIES LINK_WHAT_YOU_USE TRUE)
