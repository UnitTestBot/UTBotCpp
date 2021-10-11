/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "pointer_return.h"
#include <stdlib.h>

long long* returns_pointer_with_min(long long a, long long b) {
    static long long return_val;
    if (a < b) {
        return_val = a;
    } else {
        return_val = b;
    }

    return (&return_val);
}

unsigned int* returns_pointer_with_max(unsigned int a, unsigned int b) {
    unsigned int *return_val = (unsigned int*)malloc(sizeof(unsigned int));
    if (a > b) {
        *return_val = a;
    } else {
        *return_val = b;
    }

    return return_val;
}

int* five_square_numbers(int from) {
    static int sq[5];

    for (int i = 0; i < 10; i++) {
        sq[i] = from * from;
        from++;
    }

    return sq;
}

struct MinMax* returns_struct_with_min_max(int a, int b) {
    struct MinMax *min_max = (struct MinMax*)malloc(sizeof(struct MinMax));
    if (a < b) {
        min_max->a = a;
        min_max->b = b;
    } else {
        min_max->b = a;
        min_max->a = b;
    }

    return min_max;
}

void * return_array_like_void_ptr() {
    void * ptr = malloc(10);
    ((char*)ptr)[0] = 0;
    return ptr;
}
