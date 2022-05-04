/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "pair.h"

struct pair {
    int x, y;
};

int sum(struct pair *p) {
    if (p) {
        return p->x + p->y;
    }
    return 0;
}