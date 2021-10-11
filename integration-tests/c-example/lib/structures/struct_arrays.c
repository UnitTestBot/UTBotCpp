/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "struct_arrays.h"


int index_of_needed_struct(struct CharAndInt *arr) {
    for (int i = 0; i < 10; i++) {
        if (arr[i].c == 'c' && arr[i].x == 128) {
            return i;
        }
    }

    return -1;
}

int index_of_struct_with_equal_fields(struct Trio arr []) {
    for (int i = 0; i < 10; i++) {
        if (arr[i].a == arr[i].b) {
            if (arr[i].b == arr[i].c) {
                return i; 
            }
        }
    }

    return -1;
}
