cmake_minimum_required(VERSION 3.10)

project(installed)

find_package(Z3)

message(STATUS "Z3_FOUND: ${Z3_FOUND}")
message(STATUS "Found Z3 ${Z3_VERSION_STRING}")
message(STATUS "Z3_DIR: ${Z3_DIR}")

add_executable(installed main.c)

target_include_directories(installed PRIVATE ${Z3_C_INCLUDE_DIRS})
target_link_libraries(installed PRIVATE ${Z3_LIBRARIES})