/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef SIMPLE_TEST_PROJECT_STRUCTS_WITH_POINTERS_H
#define SIMPLE_TEST_PROJECT_STRUCTS_WITH_POINTERS_H


struct StructWithCharPointer {
    int a;
    char * str;
};


struct List {
    struct List * next;
    int val;
};

struct StructWithPointerInField {
    long long ll;
    unsigned short sh;
    struct StructWithCharPointer a;
};

int list_sum(struct List* head);

int handle_struct_with_char_ptr(struct StructWithCharPointer st);

int list_sum_sign(struct List *head);

int unsafe_next_value(struct List *node);

long long sum_of_all_fields_or_mult(struct StructWithPointerInField st, int a);

struct StructWithPointerInField some_calc(int a, int b);
#endif // SIMPLE_TEST_PROJECT_STRUCTS_WITH_POINTERS_H