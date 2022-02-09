/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_TREE_H
#define UNITTESTBOT_TREE_H

#include <stddef.h>

struct Tree {
    struct Tree *left, *right;
};

int deep(struct Tree *root);

#endif
