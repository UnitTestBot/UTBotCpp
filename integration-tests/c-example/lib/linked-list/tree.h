#ifndef TREE_H
#define TREE_H

#include <stddef.h>
#include <assert.h>

struct Tree {
    struct Tree *left, *right;
};

int deep(struct Tree *root);

#endif
