/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "floats_special.h"
#include "math.h"

int is_nanf(float x) {
    if (x != x) {
        return 1;
    } else {
        return 0;
    }
}

int is_nan(double x) {
    if (x != x) {
        return 1;
    } else {
        return 0;
    }

}

int is_inf(float x) {
    if (isinf(x)) {
        return 1;
    } else {
        return 0;
    }
}