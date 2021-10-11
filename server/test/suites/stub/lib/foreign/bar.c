/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "bar.h"
#include "../calc/sum.h"

int f(int a, int b) {
    if (sum(a, b) == a + b) {
        return 1;
    } else {
        return 2;
    }
}