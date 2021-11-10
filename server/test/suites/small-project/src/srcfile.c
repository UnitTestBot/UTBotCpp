/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */
#include "srcfile.h"
#include "libfile.h"

int f(int x) {
    if (my_sqr(x) == 1) {
        return 2;
    } else {
        return 3;
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        return f(2);
    } else {
        return f(argc);
    }
}