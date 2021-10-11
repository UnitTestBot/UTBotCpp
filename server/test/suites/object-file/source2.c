/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (f(argc) < 5) {
        return 0;
    } else {
        return atoi(argv[0]);
    }
}