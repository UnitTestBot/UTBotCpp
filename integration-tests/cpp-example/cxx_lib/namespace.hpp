/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef NAMESPACE_HPP
#define NAMESPACE_HPP

namespace uni {
    namespace inner1 {
        union U {
            int k;
            char b;
        };
    }
}

namespace str {
    struct B {
        char c;
        double y;
    };
}

namespace test {
    struct A {
        int x;
        str::B ex;
        uni::inner1::U u;
    };
}

test::A example_namespace(int a, int b);

struct StructWithUnion {
    union InnerUnion {
        char c;
        int x;
    } un;

    struct InnerStructWithUnion {
        union Inner2Union {
            char c;
            int x;
        } un2;
    } is;

    int x;
};

StructWithUnion struct_with_union_as_return_type_cpp(int t);

struct A1 {
    union B1 {
        union A1 {
            int x;
        } a1;
    } b1;

    union C1 {
        int x;
    } c1;
};

A1 multi_union(int t);

#endif
