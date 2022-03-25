/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_LINKED_LIST_H
#define UNITTESTBOT_LINKED_LIST_H

#include <stddef.h>

struct Node {
    int x;
    struct Node *next;
};

int length_of_linked_list3(struct Node *head);

int length_of_linked_list2(struct Node *head);

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

int hard_length2(struct HardNode *head);

struct Kuku {
    struct Ququ {
        struct Kuku *next;
        char letter;
    } in;
    int x;
};

int middle_length2(struct Kuku *head);

int cycle_list3(struct Node *head);

int len_bound(struct Node *head, int bound);

int sort_list(struct Node *head);

int sort_list_with_comparator(struct Node *head, int (*cmp) (int, int));

#endif
