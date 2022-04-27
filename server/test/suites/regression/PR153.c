/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include <assert.h>

typedef int (*comparison_function)(void const *, void const *);
void unused(comparison_function cmp) {
    assert(0);
}