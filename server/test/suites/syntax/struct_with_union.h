#ifndef SIMPLE_TEST_PROJECT_STRUCT_WITH_UNION_H
#define SIMPLE_TEST_PROJECT_STRUCT_WITH_UNION_H

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

struct StructWithUnionInUnion {
    union Union1 {
        int x;
        union Union2 {
            double y;
            char c;
        } t;
        char c;
    } un;
};

struct StructWithStructInUnion {
    union DeepUnion {
        int x;
        struct DeepStruct {
            char c;
            double d;
        } ds;
        long long *ptr;
    } un;
};

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

struct StructWithUnion struct_with_union_as_return_type(int t);

struct StructWithUnionInUnion struct_with_union_in_union_as_return_type(int a, int b);

struct StructWithStructInUnion struct_with_struct_in_union_as_return_type(int a, int b);

struct StructWithUnionOfUnnamedType struct_with_union_of_unnamed_type_as_return_type(int a, int b);

#endif
