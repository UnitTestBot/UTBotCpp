/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "pointer_parameters.h"

int c_strcmp(const char* a, const char *b) {
    for (int i = 0; ; i++) {
        if (a[i] != b[i]) {
            return 0;
        } else {
            if (a[i] == '\0' || b[i] == '\0') {
                return a[i] == '\0' && b[i] == '\0';
            }
        }
    }
}

int ishello(const char* a) {
    return c_strcmp(a, "hello");
}

int isworld(const unsigned char* a) {
    return c_strcmp(a, "world");
}

int longptr_cmp(long *a, long *b) {
    return (*a == *b);
}

int void_pointer_int_usage(void *x) {
    int *a = x;
    return a[0];
}

int void_pointer_char_usage(void *x) {
    char *a = x;
    return c_strcmp(a, "hello");
}

static int accept_const_void_ptr_ptr(const void **p) {
    return 0;
}

int void_ptr(int a, int b, void *ptr) {
    if (a + b > 0) {
        return 1;
    } else if (a + b < -5) {
        return -1;
    } else {
        return 0;
    }
}
