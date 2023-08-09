#ifndef SIMPLE_TEST_PROJECT_COMPLEX_STRUCTS_H
#define SIMPLE_TEST_PROJECT_COMPLEX_STRUCTS_H

struct StructWithArrays {
    int *ints[12];
    int a[10];
};

struct StructWithFunctionPointer {
    int s;
    struct FunctionPointer {
        void *(*plain) (int);
    } fptr;
};

struct One {
    int a;
    char str[12];
};

struct Two {
    int ints[5];
};

struct Three {
    char chs [2];
    int b;
    struct Two in;
};

int struct_has_alphabet(struct One st);

char arrays_in_inner_structs(struct Three st);

struct One alphabet(int a);

int st_access_s(struct StructWithFunctionPointer st);

#endif // SIMPLE_TEST_PROJECT_COMPLEX_STRUCTS_H
