add_library(literals)
target_include_directories(literals PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(literals PRIVATE foo.c PUBLIC foo.h)
target_link_libraries(literals calc)