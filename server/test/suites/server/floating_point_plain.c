/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "floating_point_plain.h"

int plain_isnan(float x) {
    if (x != x) {
        return 1;
    } else {
        return 0;
    }
}