/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Refs.h"

int Refs::foo(int& a) {
    a++;
    if (a > 5) {
        return 10;
    }
    return 20;
}

int& Refs::bar(char c) {
    intRef = 5;
    return intRef;
}