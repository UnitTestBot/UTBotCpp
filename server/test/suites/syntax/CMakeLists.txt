cmake_minimum_required(VERSION 3.10)
project(syntax1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-O2)

add_executable(syntax1
        pointer_parameters.c
        pointer_return.c
        types.c
        simple_structs.c
        simple_unions.c
        complex_structs.c
        packed_structs.c
        typedefs_1.c
        typedefs_2.c
        enums.c
        constants.c
        void_functions.c
        qualifiers.c
        structs_with_pointers.c
        struct_with_union.c
        functions_as_params.c
        multi_arrays.c
        variadic.c
        main.c
        types_3.c
        floats_special.c
        linked_list.c
        tree.c
        different_parameters.cpp
        simple_class.cpp
        inner_unnamed.c
        array_sort.c
        constructors.cpp
        stubs.c)
