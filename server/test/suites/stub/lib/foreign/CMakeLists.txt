add_library(foreign)
target_include_directories(foreign PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(foreign PRIVATE bar.c PUBLIC bar.h)
