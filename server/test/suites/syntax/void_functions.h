/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef SIMPLE_TEST_PROJECT_VOID_FUNCTIONS_H
#define SIMPLE_TEST_PROJECT_VOID_FUNCTIONS_H

struct ThisStruct {
    int a;
};

void print_sign(int a);

void print_signs_for_two_structs(struct ThisStruct thisStr1, struct ThisStruct thisStr2);

#endif //SIMPLE_TEST_PROJECT_VOID_FUNCTIONS_H