/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include <unistd.h>

int sleepy(int x) {
    if (x > 0) {
        sleep(50);
        if (x == 0) {
            return 0;
        }
        return -1;
    } else {
        return 1;
    }
}
