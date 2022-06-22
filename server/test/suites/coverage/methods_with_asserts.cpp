/*
* Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "methods_with_asserts.h"
#include <cassert>

int first_function(int a) {
    if (a < 5) {
        assert(a > 5);
    } else {
        return 5;
    }
    return 1;
}

int second_function(int a) {
    if (a < 0) {
        assert(false);
    } else {
        return 5;
    }
}

int get_sign(int a) {
    int sign = 0;
    if (a > 0) {
        sign = 1;
    } else if (a == 0) {
        sign = 0;
    } else {
        sign = -1;
    }
    assert(a * sign > 0);
    return 5;
}

int third_function(int *a) {
    if (*a > 0) {
        assert(*a < 0);
    } else {
        assert(*a > 0);
    }
    return 0;
}

int fourth_function(int a) {
    if (a > 0) {
        assert(a > 0);
    } else {
        assert(a <= 0);
    }
    return 5;
}
