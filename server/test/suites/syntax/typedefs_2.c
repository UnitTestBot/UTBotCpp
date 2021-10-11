/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "typedefs_2.h"


int enumSign1ToInt(Sign1 s) {
    if (s == ZER1) {
        return 0;
    }
    if (s == NEG1) {
        return -1;
    } else {
        return 1;
    }
}

Sign1 intToSign1(int a) {
    if (a == 0) {
        return ZER1;
    }

    if (a > 0) {
        return POS1;
    }
    return NEG1;
}


int enumSign2ToInt(Sign2 s) {
    if (s == ZER2) {
        return 0;
    }
    if (s == NEG2) {
        return -1;
    } else {
        return 1;
    }
}

Sign2 intToSign2(int a) {
    if (a == 0) {
        return ZER2;
    }

    if (a > 0) {
        return POS2;
    }
    return NEG2;
}