/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stddef.h>

struct Node {
    int x;
    struct Node *next;
};

struct HardNode {
    struct Inner {
        int x;
        struct SuperInner {
            struct HardNode *next;
            int step;
        } superIn;
        double d;
    } in;
    char letter;
    struct HardNode *randomRef;
};

struct Kuku {
    struct Ququ {
        struct Kuku *next;
        char letter;
    } in;
    int x;
};

//Don't cover branch with NULL
int length_of_linked_list3(struct Node *head);

//Don't cover branch with NULL
int length_of_linked_list2(struct Node *head);

int sum_list(struct Node *head);

//Wait fix from Alexandr
int hard_length2(struct HardNode *head);

int middle_length2(struct Kuku *head);

int cycle_list3(struct Node *head);

int len_bound(struct Node *head, int bound);

int sort_list(struct Node *head);

#endif
