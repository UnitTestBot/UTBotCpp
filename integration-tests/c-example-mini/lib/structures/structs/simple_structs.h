#ifndef SIMPLE_TEST_PROJECT_SIMPLE_STRUCTS_H
#define SIMPLE_TEST_PROJECT_SIMPLE_STRUCTS_H

struct MyStruct {
    short x;
    const int a;
};

struct OneMoreStruct {
    int a;
    unsigned short c;
    long long b;
};

struct StructWithChars {
    char a;
    signed char c;
    unsigned char u;
    int b;
};

struct MainStruct {
    struct InnerStruct {
        struct InInnerStruct {
            unsigned int u;
            long long l; 
        };
        char c;
        struct InInnerStruct ininner;
        short s;
    } inner;
    
    int x;
    // struct InnerStruct inner;
    long long y;
};

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

struct A {
    union B {
        struct C {
            union D {
                char c;
                int x;
                double y;
            } arr;
        } t;
    } a;
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


int get_sign_struct(struct MyStruct st);

int calculate_something(struct OneMoreStruct str);

char get_symbol_by_struct(const struct StructWithChars st);

signed char operate_with_inner_structs(struct MainStruct st);

struct MainStruct struct_as_return_type(int a);

struct StructWithUnion struct_with_union_as_return_type(int t);

struct A complex_struct_with_union_as_return_type(int t);

struct StructWithUnionInUnion struct_with_union_in_union_as_return_type(int a, int b);

struct StructWithStructInUnion struct_with_struct_in_union_as_return_type(int a, int b);

#endif // SIMPLE_TEST_PROJECT_SIMPLE_STRUCTS_H
