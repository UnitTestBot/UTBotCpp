/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "simple_calc.h"
#include "module/libfunc.h"

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

