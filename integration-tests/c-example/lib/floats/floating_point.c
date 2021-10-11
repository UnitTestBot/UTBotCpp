/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "floating_point.h"
#include "math.h"

int get_double_sign(double x) {
    if (x == 0.0) {
        return 0;
    }
    if (x > 0) {
        return 1;
    } else {
        return -1;
    }
}

int is_close(float_t x, int y) {
    if (isnan(x)) {
        return 0;
    }
    if (fabs(x - y) < 1.0) {
        return 1;
    } else {
        return 0;
    }
}

float long_double_arith(long double x) {
    x *= 2;
    x -= 3.21;
    x *= fabsl(x);
    if (x == 1.0) {
        return 1.0;
    } else {
        return 3.5;
    }
}

int array_max(float arr []) {
    float mx = arr[0];
    for (int i = 1; i < 3; i++) {
        if (mx < arr[i]) {
            mx = arr[i];
        }
    }
    float eps = 1e-6;
    if (mx < -eps) {
        return -1;
    }
    if (mx > eps) {
        return 1;
    }
    return 0;
}

struct FParray fp_array(int a) {
    if (a < 0) {
        struct FParray res = {{1.23, 3.21}};
        return res;
    }
    struct FParray res = {{12.3, 32.1}};
    return res;
}