add_library(issue-140)
target_include_directories(issue-140 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ALL_SOURCES "*.c" "*.h")
target_sources(issue-140 PRIVATE ${ALL_SOURCES})
