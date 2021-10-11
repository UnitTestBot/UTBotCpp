/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "types.h"

char a_or_b(char a, char b) {
    if (a == 'a') {
        return a;
    }
    if (b == 'b') {
        return b;
    }

    if (a > b) {
        return a;
    }
    return b;
}

signed long long int max_long(long long a, signed long long b) {
    if (a > b) {
        return a;
    }
    return b;
}

short min_short(short a, short b) {
    if (a < b) {
        return a;
    }

    return b;
}

short int min_divided_by_2(signed short a, signed short int b) {
    return min_short(a, b) / 2;
}

signed char some_func(char a, unsigned char b) {
    if (b == 'z' && a > b) return a;
    if (b != 'z') return b;
    return '0';
}

int fun_that_accept_bools(_Bool a, bool b) {
    if (a && b) return 1;
    if (a) return 2;
    if (b) return 3;
    return 4;
}

bool is_positive(int arg) {
    if (arg > 0) return true;
    return false;
}