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

struct MinMax* returns_struct_with_min_max(int a, int b);

#endif // SIMPLE_TEST_PROJECT_POINTER_RETURN_H
