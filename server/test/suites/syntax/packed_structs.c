/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "packed_structs.h"

int get_sign_packedStruct1(struct PackedStruct1 st) {
    if (st.i == 0) {
        return 0;
    }
    if (st.i > 0) {
        return 1;
    } else {
        return -1;
    }
}


char get_val_by_packedStruct2(struct PackedStruct2 st) {
    if (st.c == st.s.c) {
        if (st.i > st.s.i) {
            return '1';
        }
        return '2';
    }

    if (st.i == st.s.i) {
        return '3';
    }

    return '4';
}

short get_val_by_otherPackedStruct(struct OtherPackedStruct st) {
    if (st.a == st.b) {
        if (st.str.c == '!') {
            return 0;
        }
        return st.s;
    }
    if (st.str.c == 'S') {
        return 5;
    }

    return -1;
}