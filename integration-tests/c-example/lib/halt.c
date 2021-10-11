/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "halt.h"

#include "signal.h"

int raise_by_num(int num) {
    return raise(num);
}

int raise_stop(int _) {
    return raise(SIGSTOP);
}