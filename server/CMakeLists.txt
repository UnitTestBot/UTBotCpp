cmake_minimum_required(VERSION 3.12)

if (NOT DEFINED VERSION)
    if (DEFINED ENV{VERSION})
        set(VERSION $ENV{VERSION})
    else ()
        set(VERSION "0.0.1")
    endif ()
endif ()

if (NOT DEFINED RUN_INFO)
    if (DEFINED ENV{RUN_INFO})
        set(RUN_INFO $ENV{RUN_INFO})
    else ()
        set(RUN_INFO "local")
    endif ()
endif ()

project(UTBotCpp DESCRIPTION "Tool that generates unit test by C/C++ source code, trying to reach all branches and maximize code coverage"
        HOMEPAGE_URL "https://unittestbot.org" VERSION ${VERSION})
set(PROJECT_ORG "UnitTestBot")

message("Project: ${PROJECT_NAME}")
message("Organisation: ${PROJECT_ORG}")
message("Homepage: ${CMAKE_PROJECT_HOMEPAGE_URL}")
message("Version: ${CMAKE_PROJECT_VERSION}")
message("Run from: ${RUN_INFO}")
include_directories(${PROJECT_BINARY_BIN})

configure_file(config.h.in config.h @ONLY)

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
        "${PROJECT_SOURCE_DIR}/src/streams/coverage/*"
        "${PROJECT_SOURCE_DIR}/src/streams/stubs/*"
        "${PROJECT_SOURCE_DIR}/src/streams/tests/*"
        "${PROJECT_SOURCE_DIR}/src/testgens/*"
        "${PROJECT_SOURCE_DIR}/src/utils/*"
        "${PROJECT_SOURCE_DIR}/src/utils/stats/*"
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

option(ENABLE_PRECOMPILED_HEADERS "Enable precompiled headers" ON)

if (ENABLE_PRECOMPILED_HEADERS)
    set_source_files_properties(${ALL_PROTO_GENERATED_SOURCES} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif ()

################################################################################
# Library
################################################################################
add_library(UTBotCppLib STATIC ${ALL_PROTO_GENERATED_SOURCES} ${ALL_SOURCES})
target_include_directories(UTBotCppLib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        resources
        thirdparty/ordered-map)

target_link_libraries(UTBotCppLib PUBLIC clangTooling clangBasic clangASTMatchers clangRewriteFrontend
        gRPC::grpc++_reflection
        gRPC::grpc++
        protobuf::libprotobuf
        loguru
        kleeRunner
        )
if (ENABLE_PRECOMPILED_HEADERS)
    target_precompile_headers(UTBotCppLib PUBLIC pch.h)
endif ()

################################################################################
# Executable
################################################################################
add_llvm_executable(utbot main.cpp)
target_link_libraries(utbot PUBLIC
        UTBotCppLib
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
            gtest
            UTBotCppLib
    )

    if (ENABLE_PRECOMPILED_HEADERS)
        target_precompile_headers(UTBot_UnitTests PRIVATE [["gtest/gtest.h"]])
    endif ()

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
