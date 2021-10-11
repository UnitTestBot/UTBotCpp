cmake_minimum_required(VERSION 3.10)

project(timeout_project)

add_executable(exe main.c)
target_compile_options(exe PUBLIC -std=gnu99)