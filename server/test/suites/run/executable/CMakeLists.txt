cmake_minimum_required(VERSION 3.10)

project(executable_project)

add_executable(executable main.c func.c)
target_compile_options(executable PUBLIC -std=gnu99)