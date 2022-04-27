execute_process(
    COMMAND python generate.py 50
    COMMAND python generate.py 100
    COMMAND python generate.py 500
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)


FILE(GLOB MyCSources *.c)

add_library(src ${MyCSources})
target_include_directories(src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE ALL_SOURCES "*.c" "*.h")
target_sources(src PRIVATE ${sALL_SOURCES})
