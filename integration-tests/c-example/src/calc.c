/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "calc.h"
#include "module/libfunc.h"

int calc_two_numbers(char op, int a, int b) {
    op_func f = return_op(op);
    if (f == NULL) {
        return -1;
    } else {
        return f(a, b);
    }
}

int f(int a) {
    return 3 * a;
}

int other_module_call(int a) {
    int b = libfunc(a);
    if (b > 0) {
        return 1;
    } else {
        return 2;
    }
}

int calc_two_numbers_f(char a, char b) {
    typedef int (*f_arr_type)(int);
    f_arr_type f_arr[10];
    f_arr[0] = *f;
    if (pointerToPointer(f_arr, a) == b) {
        return 1;
    }
    return 2;
}
