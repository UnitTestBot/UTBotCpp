/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "pointer_parameters.h"

int c_strcmp(const char *a, const char *b) {
    for (int i = 0;; i++) {
        if (a[i] != b[i]) {
            return 0;
        } else {
            if (a[i] == '\0' || b[i] == '\0') {
                return a[i] == '\0' && b[i] == '\0';
            }
        }
    }
}

int ishello(const char *a) {
    return c_strcmp(a, "hello");
}

int longptr_cmp(long *a, long *b) {
    return (*a == *b);
}

static int accept_const_void_ptr_ptr(const void **p) {
    return 0;
}