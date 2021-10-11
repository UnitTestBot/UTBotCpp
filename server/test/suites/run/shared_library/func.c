/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "func.h"

static _Bool compare(int a, int b) {
    return a < b;
}

int min(int a, int b) {
    if (compare(a, b)) {
        return a;
    } else {
        return b;
    }
}
