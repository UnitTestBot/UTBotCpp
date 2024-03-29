#ifndef SIMPLE_TEST_PROJECT_STRUCT_WITH_UNION_H
#define SIMPLE_TEST_PROJECT_STRUCT_WITH_UNION_H

struct StructWithUnionOfUnnamedType {
    union {
        int x;
        struct {
            char c;
            double d;
        } ds;
        long long *ptr;
    } un;
};

struct StructWithAnonymousUnion {
    union {
        int x;
        struct {
            char c;
            double d;
        };
        long long *ptr;
    };
};

struct StructWithUnionOfUnnamedType struct_with_union_of_unnamed_type_as_return_type(int a, int b);

struct StructWithAnonymousUnion struct_with_anonymous_union_as_return_type(int a, int b);

#endif // SIMPLE_TEST_PROJECT_STRUCT_WITH_UNION_H
