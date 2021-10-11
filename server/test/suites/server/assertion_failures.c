/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "assertion_failures.h"

int buggy_function2(int a) {
    assert(a == 7);
    assert(a < 7);
    return a;
}
