/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "namespace.hpp"

test::A example_namespace(int a, int b) {
    test::A res;
    if (a + b > 7) {
        res.x = 5;
        res.ex.c = 't';
        res.ex.y = 1e5;
        res.u.k = 17;
    } else if (a + b < -3) {
        res.x = -1;
        res.ex.c = 'u';
        res.ex.y = -1e2;
        res.u.k = 101;
    } else {
        res.x = 10;
        res.ex.c = 'z';
        res.ex.y = 1.23;
        res.u.b = 'j';
    }
    return res;
}

StructWithUnion struct_with_union_as_return_type_cpp(int t) {
    StructWithUnion st;
    if (t == 0) {
        st.un.c = 'a';
        st.is.un2.x = 101;
        st.x = 155;
    } else {
        st.un.x = 17;
        st.is.un2.c = '0';
        st.x = -108;
    }
    return st;
}

A1 multi_union(int t) {
    A1 res;
    if (t == 0) {
        res.b1.a1.x = 10;
        res.c1.x = 9;
    } else {
        res.b1.a1.x = 5;
        res.c1.x = 6;
    }
    return res;
}
