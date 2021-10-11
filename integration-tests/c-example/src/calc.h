/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef CALC_H
#define CALC_H

#include "pointers/function_pointers.h"
#include <stdlib.h>

int calc_two_numbers(char op, int a, int b);

int calc_two_numbers_f(char a, char b);

int f(int a);

int other_module_call(int a);
#endif //CALC_H
