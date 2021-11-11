/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef SIMPLE_TEST_PROJECT_SIMPLE_UNIONS_H
#define SIMPLE_TEST_PROJECT_SIMPLE_UNIONS_H

#include "assert.h"

union IntBytesUnion {
    char bytes[4];
    int number;
};

union ShortBytesUnion {
    char bytes[2];
    short number;
};

union SuperFloat {
    float f;
    int i;
};

union Vector2D {
    int m[2];
    struct {
        int x, y;
    };
};

union Heterogeneous {
    int a;
    long long b;
    unsigned short c;
};

union MainUnion {
    union InnerUnion {
        union InInnerUnion {
            unsigned int u;
            long long l;
        };
        char c;
        union InInnerUnion ininner;
        short s;
    } inner;

    int x;
    // union InnerUnion inner;
    long long y;
};

typedef enum {
    CHAR_TAG,
    INT_TAG
} VariantTag;

struct Variant {
    VariantTag tag;
    union VariantData {
        char c;
        int i;
    } data;
};

union UnionWithPointer {
    int * a;
    long long b;
};

int get_sign_union(union IntBytesUnion st);

int extract_bit(union ShortBytesUnion x);

int calculate_something_union(union Heterogeneous str);

int get_coordinate(union Vector2D vect, int i);

signed char operate_with_inner_unions(union MainUnion st);

union MainUnion union_as_return_type(int a);

int to_int(struct Variant variant);

int sumOfUnionArray(union IntBytesUnion u[5]);

long long operateWithUnionWithPointer(union UnionWithPointer un);
#endif //SIMPLE_TEST_PROJECT_SIMPLE_UNIONS_H
