add_library(c_lib)
target_include_directories(c_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ALL_SOURCES "*.c" "*.h")
target_sources(c_lib PRIVATE ${ALL_SOURCES})