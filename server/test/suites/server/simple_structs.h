#ifndef SIMPLE_TEST_PROJECT_SIMPLE_STRUCTS_H
#define SIMPLE_TEST_PROJECT_SIMPLE_STRUCTS_H

struct MyStruct {
    short x;
    int a;
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

int get_sign_struct(struct MyStruct st);

int calculate_something(struct OneMoreStruct str);

char get_symbol_by_struct(struct StructWithChars st);

signed char operate_with_inner_structs(struct MainStruct st);

struct MainStruct struct_as_return_type(int a);

#endif // SIMPLE_TEST_PROJECT_SIMPLE_STRUCTS_H
