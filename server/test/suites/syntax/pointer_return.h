/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef SIMPLE_TEST_PROJECT_POINTER_RETURN_H
#define SIMPLE_TEST_PROJECT_POINTER_RETURN_H

long long* returns_pointer_with_min(long long a, long long b);

unsigned int* returns_pointer_with_max(unsigned int a, unsigned int b);

int* five_square_numbers(int from);

struct MinMax {
    int a;
    int b;
};

struct SupportedStruct4 {
    char* c;
};

struct MinMax* returns_struct_with_min_max(int a, int b);

const char* return_const_char(int a);

char const * return_char_const_pointer(int a);

const struct MinMax* returns_const_struct_with_min_max(int a, int b);

int * return_nullptr(int x);

const struct SupportedStruct4* return_null_struct(int x);


#endif // SIMPLE_TEST_PROJECT_POINTER_RETURN_H
