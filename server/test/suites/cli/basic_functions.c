/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "basic_functions.h"

int max_(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int sqr_positive(int a) {
    if (a < 0) {
        return -1;
    } else {
        return a * a;
    }
}

int simple_loop(unsigned int n) {
    int i = 0;
    while (i < n) {
        i = i + 1;
        if (n % i == 37)
            return 1;
        else if (i == 50)
            return 2;
    }
    return 0;
}

const char* const_str(int x) {
    return "abacaba";
}
