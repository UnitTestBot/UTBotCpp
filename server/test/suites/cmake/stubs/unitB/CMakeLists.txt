add_library(unitB)
target_include_directories(unitB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(unitB PRIVATE callee.c PUBLIC callee.h)
