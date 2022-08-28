
add_library(unitA caller.c)
target_include_directories(unitA PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(unitA unitB)