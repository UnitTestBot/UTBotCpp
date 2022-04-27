/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */


#include "pair.h"

struct double_pair {
    struct pair *p1;
    struct pair *p2;
};

int sum_double_pairs(struct double_pair *p) {
    return sum(p->p1) + sum(p->p2);
}
