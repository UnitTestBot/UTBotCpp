/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "qualifiers.h"

#include <stdlib.h>
#include <string.h>

int c_strcmp_2(const  char * restrict const a, const char * restrict const b) {
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

int ishello_2(char * restrict a) {
    return c_strcmp_2(a, "hello");
}


const long long * const returns_pointer_with_min_modifier(const long long a, const long long b) {
    static long long return_val;
    if (a < b) {
        return_val = a;
    } else {
        return_val = b;
    }

    return (&return_val);
}

char * const foo__(int const a) {
    if (a < 0) {
        return "-1";
    } else if (a == 0) {
        return "0";
    } else {
        return "1";
    }
}


const char * const foo_bar(volatile int a) {
    if (a < 0) {
        return "-1";
    } else if (a == 0) {
        return "0";
    } else {
        return "1";
    }
}


const struct MinMaxQ * restrict const returns_struct_with_min_max_Q(volatile const int a, const volatile int b) {
    struct MinMaxQ * const min_max = (struct MinMaxQ* const)malloc(sizeof(struct MinMaxQ));
    if (a < b) {
        min_max->a = a;
        min_max->b = b;
        strcpy(min_max->chars, "ab");
    } else {
        min_max->b = a;
        min_max->a = b;
        strcpy(min_max->chars, "ba");
    }

    return min_max;
}