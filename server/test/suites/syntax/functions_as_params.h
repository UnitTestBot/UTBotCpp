/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef SIMPLE_TEST_PROJECT_FUNCTIONS_AS_PARAMS_H
#define SIMPLE_TEST_PROJECT_FUNCTIONS_AS_PARAMS_H

#include "simple_structs.h"

int receiver(int (*f)(int, int), char c);

char* pointerParam(char* (*f)(int*), int* x);

int structParam(int (*f)(struct MyStruct), const char* s);

int structPointerParam(int (*f)(struct MyStruct*), const int* arr);

typedef int (*op_func)(int, int);
struct FStruct {
    int a;
    op_func f;
};

int calcFunctionStruct(struct FStruct function_struct, int b);


#endif //SIMPLE_TEST_PROJECT_FUNCTIONS_AS_PARAMS_H