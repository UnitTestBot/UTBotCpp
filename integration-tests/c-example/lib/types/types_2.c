/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "types_2.h"

size_t foo(MY_INT x) {
    if (x < 0) {
        return 0;
    } else {
        return 1;
    }
}


MY_INT bar(size_t a, MY_INT b) {
    if (a < 0 && b < 0) {
        return -2;
    }

    if (a <= 0) {
        return -1;
    }

    if (b >= 0) {
        return 0; 
    }

    return 1;
}

typedef unsigned int typedef_from_c;
typedef int typedef_from_c_param;

// UTBot should use types from header
// declaration and not from definiton 
typedef_from_c diff(typedef_from_c_param x) {
    if (x == 2) {
        return 2;
    }

    return 1;
}

