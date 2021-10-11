add_library(cxx_lib)
target_include_directories(cxx_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ALL_SOURCES "*.cpp" "*.h"  "*.hpp")
target_sources(cxx_lib PRIVATE ${ALL_SOURCES})