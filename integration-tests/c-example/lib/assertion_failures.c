/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "assertion_failures.h"

int buggy_function1(int a, int b) {
    if (a > b) {
        assert(a != 42);
        return a;
    }
    else {
        return b;
    }
}

int buggy_function2(int a) {
    assert(a == 7);
    assert(a < 7);
    return a;
}

int buggy_function3(int a, int b) {
    assert(a < 1);
    assert(a < 10); // Can't generate test that fails assertion since if a >= 10 then the first assertion fails
    return a;
}
