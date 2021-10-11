add_library(calc)
target_include_directories(calc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(calc PRIVATE sum.c mult.c PUBLIC sum.h mult.h)