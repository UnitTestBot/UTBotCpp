/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef TREE_H
#define TREE_H

#include <stddef.h>
#include <assert.h>

struct Tree {
    struct Tree *left, *right;
};

int deep(struct Tree *root);

#endif