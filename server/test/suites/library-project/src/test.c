/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "test.h"

int test(int x) {
    if (sum(x, 2 * x) > 5) {
        return 1;
    } else {
        return -1;
    }
}
