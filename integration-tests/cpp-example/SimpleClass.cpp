/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "SimpleClass.h"

SimpleClass::SimpleClass() {
    c = 10;
}

int SimpleClass::sum_with_c(int a) {
    if (a % 2 == 0) {
        return a + c;
    }
    return 17;
}

int SimpleClass::sub_with_c(int a, int b) {
    if (a == c) {
        c++;
        return a - c;
    }
    return a - b;
}

int SimpleClass::mul_with_c(int a, int b) {
    if (a != 0 && b != 0) {
        return a * b * c;
    }
    return 2;
}

int outer_func(int a, int b) {
    if (a - b != 12) {
        return a * a - 2 * a * b + b * b;
    }
    return 21;
}