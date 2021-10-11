#include "tree.h"

int deep(struct Tree *root) {
    if (root == NULL) {
        return 0;
    }
    if (root->left != NULL && root->right != NULL && root->left->right == root->right && root->right->left == root->left) {
        return 1;
    } else {
        return -1;
    }
}

