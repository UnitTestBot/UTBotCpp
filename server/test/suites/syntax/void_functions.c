/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "void_functions.h"
#include <stdio.h>

void print_sign(int a) {
    if (a < 0) {
        printf("'a' is negative");
    } else if (a > 0) {
        printf("'a' is positive");
    } else {
        printf("'a' is zero");
    }
}


void print_signs_for_two_structs(struct ThisStruct thisStr1, struct ThisStruct thisStr2) {
    if (thisStr1.a * thisStr2.a > 0) {
        printf("Equal signs\n");
    } else if (thisStr1.a * thisStr2.a < 0) {
        printf("Different signs\n");
    } else {
        printf("At least one is 0\n");
    }
}

void no_parameters() {
    return;
}

int void_parameter(void) {
    return 6;
}