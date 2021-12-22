add_library(lib)
target_include_directories(lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ALL_SOURCES "*.c" "*.h")
target_sources(lib PRIVATE ${ALL_SOURCES})