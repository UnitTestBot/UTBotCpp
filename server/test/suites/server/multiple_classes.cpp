/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "multiple_classes.h"

int first_class::get1() {
    return 1;
}

int second_class::get2() {
    return 2;
}

int second_class::third_class::get3() {
    return 3;
}

