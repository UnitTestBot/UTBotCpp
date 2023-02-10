#ifndef UTBOTCPP_HARD_LINKED_LIST_H
#define UTBOTCPP_HARD_LINKED_LIST_H

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
