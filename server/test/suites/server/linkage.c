/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

_Bool externed_global = 0;

int ordinary_sum(int a, int b) {
    return a + b;
}

extern int extern_sum(int a, int b) {
    return a + b;
}

static int static_sum(int a, int b) {
    return a + b;
}