cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-O2)

add_executable(server
        inner/inner_basic_functions.c
        assertion_failures.c
        basic_functions.c
        dependent_functions.c
        different_variables.c
        floating_point.c
        floating_point_plain.c
        globals.c
        keywords.c
        linkage.c
        main.c
        multi_dim_pointers.c
        pointer_parameters.c
        pointer_return.c
        simple_structs.c
        simple_unions.c
        struct_with_union.c
        structs_with_pointers.c
        thread_local.c
        complex_structs.c
        typedefs.c
        types.c
        alignment.c
        symbolic_stdin.c
        multiple_classes.cpp
        file.c
        hard_linked_list.c
        input_output.c
        linked_list.c
        tree.c
        )
