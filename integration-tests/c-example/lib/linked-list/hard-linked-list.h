#ifndef HARD_LINKED_LIST_H
#define HARD_LINKED_LIST_H

struct Data {
    int x;
    int y;
};

struct Node {
    int *x;
    struct Data *data;
    struct Node *next;
};

#endif
