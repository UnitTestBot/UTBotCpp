/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "typedefs_1.h"

int gid_to_name (gid_t gid) {
    if (gid == 2) return 2;
    return 3;
}


int sign_of_typedef_struct(TypeDefStruct x) {
    if (x.a > 0) {
        return 1;
    }

    if (x.a < 0) {
        return -1;
    }

    return 0;
}

int sign_of_typedef_struct2(TypeDefStruct2 x) {
    if (x.a > 0) {
        return 1;
    }

    if (x.a < 0) {
        return -1;
    }

    return 0;
}

size_t min_size_t(size_t a, size_t b) {
    if (a < b) {
        return a;
    }

    return b;
}

size_t_alias min_size_t_alias(size_t_alias a, size_t_alias b) {
    if (a < b) {
        return a;
    }
    return b;
}


int sign_of_typedef_union(TypeDefUnion x) {
    if (x.a > 0) {
        return 1;
    }

    if (x.a < 0) {
        return -1;
    }

    return 0;
}

int sign_of_typedef_union2(TypeDefUnion2 x) {
    if (x.a > 0) {
        return 1;
    }

    if (x.a < 0) {
        return -1;
    }

    return 0;
}