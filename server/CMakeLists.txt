cmake_minimum_required(VERSION 3.10)
project(UnitTestBot)

if (UNIX)
    add_compile_definitions(_LINUX)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fstandalone-debug -fno-discard-value-names -fno-elide-constructors")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")

include(cmake/build_type.cmake)

enable_testing()
add_compile_options(-frtti -fexceptions)

set(CMAKE_PREFIX_PATH $ENV{UTBOT_INSTALL_DIR})
find_package(Clang REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_CMAKE_DIR}")
include(AddLLVM)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(${CLANG_DEFINITIONS})

add_definitions(-DUTBOT_DEV_ROOT_DIR="$ENV{UTBOT_ALL}")

set(GRPC_PATH $ENV{GRPC_PATH})
set(CMAKE_PREFIX_PATH ${GRPC_PATH}/lib/cmake/grpc ${GRPC_PATH}/lib/cmake/protobuf)
if (NOT DEFINED _GRPC_CPP_PLUGIN_EXECUTABLE)
    set(_GRPC_CPP_PLUGIN_EXECUTABLE ${GRPC_PATH}/bin/grpc_cpp_plugin)
endif ()
include_directories(${GRPC_PATH}/include)

find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_definitions(-DLOGURU_WITH_STREAMS=1)
SET(GCC_COVERAGE_LINK_FLAGS "-lpthread -ldl")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

get_filename_component(UTBOT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)

set(SUBMODULES_DIR "${UTBOT_DIR}/submodules")
set(LOGGER_DIR "${SUBMODULES_DIR}/loguru")
set(LOGGER_AUX_DIR "${PROJECT_SOURCE_DIR}/logger-aux")
add_library(loguru ${LOGGER_DIR}/loguru.cpp ${LOGGER_AUX_DIR}/loguru_debug.cpp)
target_include_directories(loguru PUBLIC ${LOGGER_DIR})
include_directories(${LOGGER_AUX_DIR})

set(JSON_PATH "${SUBMODULES_DIR}/json")
include_directories(${JSON_PATH}/src)

set(TSL_PATH "${SUBMODULES_DIR}/ordered-map")
message("tsl include from here: ${TSL_PATH}/include")
include_directories(${TSL_PATH}/include)

set(PARALLEL_HASHMAP_PATH "${SUBMODULES_DIR}/parallel-hashmap")
include_directories(${PARALLEL_HASHMAP_PATH})

set(KLEE_PATH $ENV{UTBOT_ALL}/klee/)
include_directories(${KLEE_PATH}/include)

set(CLI_PATH $ENV{CLI_PATH})
include_directories(${CLI_PATH})

file(GLOB FETCHERS_SRC
        "${PROJECT_SOURCE_DIR}/src/fetchers/*"
        )

file(GLOB ALL_SOURCES
        "${PROJECT_SOURCE_DIR}/src/*"
        "${PROJECT_SOURCE_DIR}/src/building/*"
        "${PROJECT_SOURCE_DIR}/src/clang-utils/*"
        "${PROJECT_SOURCE_DIR}/src/tasks/*"
        "${PROJECT_SOURCE_DIR}/src/coverage/*"
        "${PROJECT_SOURCE_DIR}/src/environment/*"
        "${PROJECT_SOURCE_DIR}/src/exceptions/*"
        "${PROJECT_SOURCE_DIR}/src/fetchers/*"
        "${PROJECT_SOURCE_DIR}/src/printers/*"
        "${PROJECT_SOURCE_DIR}/src/streams/*"
        "${PROJECT_SOURCE_DIR}/src/sarif/*"
        "${PROJECT_SOURCE_DIR}/src/streams/coverage/*"
        "${PROJECT_SOURCE_DIR}/src/streams/stubs/*"
        "${PROJECT_SOURCE_DIR}/src/streams/tests/*"
        "${PROJECT_SOURCE_DIR}/src/testgens/*"
        "${PROJECT_SOURCE_DIR}/src/utils/*"
        "${PROJECT_SOURCE_DIR}/src/stubs/*"
        "${PROJECT_SOURCE_DIR}/src/visitors/*"
        "${PROJECT_SOURCE_DIR}/src/commands/*"
        "${PROJECT_SOURCE_DIR}/src/types/*"
        "${PROJECT_SOURCE_DIR}/resources/*"
        ${FETCHERS_SRC}
        )

set(PROTO_DIR "${PROJECT_SOURCE_DIR}/proto")
set(PROTO_COMPILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/protobuf")
file(GLOB PROTO_FILES "${PROTO_DIR}/*.proto")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/protobuf)

foreach (proto_file ${PROTO_FILES})
    get_filename_component(proto_src ${proto_file} PATH)
    get_filename_component(proto_filename ${proto_file} NAME_WE)
    execute_process(
            COMMAND protoc --grpc_out ${PROTO_COMPILE_DIR}
            --cpp_out ${PROTO_COMPILE_DIR}
            -I "${proto_src}"
            --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
            "${proto_file}")
    set_property(SOURCE main.cpp APPEND PROPERTY OBJECT_DEPENDS
            ${PROTO_COMPILE_DIR}/${proto_filename}.grpc.pb.h)
endforeach ()

file(GLOB ALL_PROTO_GENERATED_SOURCES
        ${PROTO_COMPILE_DIR}/*.cc
        ${PROTO_COMPILE_DIR}/*.h)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

find_package(run_klee REQUIRED)

add_library(UnitTestBotLib STATIC ${ALL_PROTO_GENERATED_SOURCES} ${ALL_SOURCES} src/sarif/ProjectSarif.cpp src/sarif/ProjectSarif.h)
target_include_directories(UnitTestBotLib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        resources
        thirdparty/ordered-map)

target_link_libraries(UnitTestBotLib PUBLIC clangTooling clangBasic clangASTMatchers clangRewriteFrontend
        gRPC::grpc++_reflection
        gRPC::grpc++
        protobuf::libprotobuf
        loguru
        kleeRunner
        )
add_llvm_executable(utbot main.cpp)
target_link_libraries(utbot PUBLIC
        UnitTestBotLib
        loguru
        )

################################################################################
# Testing
################################################################################
option(ENABLE_UNIT_TESTS "Enable unit tests" ON)

if (ENABLE_UNIT_TESTS)
    message(STATUS "Unit tests enabled")

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory($ENV{UTBOT_ALL}/gtest
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL)
    file(GLOB ALL_TESTS "${PROJECT_SOURCE_DIR}/test/framework/*.cpp")

    add_executable(
            UTBot_UnitTests
            ${ALL_TESTS}
    )

    target_include_directories(UTBot_UnitTests PUBLIC src src/include $ENV{UTBOT_ALL}/gtest/googletest)
    target_link_libraries(
            UTBot_UnitTests
            PUBLIC
            gtest_main
            UnitTestBotLib
    )

    add_test(NAME test COMMAND UTBot_UnitTests)
else ()
    message(STATUS "Unit tests disabled")
endif ()

################################################################################
# Miscellaneous install
################################################################################
install(TARGETS utbot
        DESTINATION ${CMAKE_INSTALL_PREFIX}
        )