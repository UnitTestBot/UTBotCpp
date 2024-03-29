#ifndef SIMPLE_TEST_PROJECT_COMPLEX_STRUCTS_H
#define SIMPLE_TEST_PROJECT_COMPLEX_STRUCTS_H

struct One {
    int a;
    char str[12];
};

struct Two {
    int ints[5];
};

struct Three {
    char chs[2];
    int b;
    struct Two in;
};

struct WithAnonymousStruct {
    struct { int x, y; };
    int m;
};

int struct_has_alphabet(struct One st);

char arrays_in_inner_structs(struct Three st);

int count_equal_members_anon_structure(struct WithAnonymousStruct st);

struct One alphabet(int a);

#endif // SIMPLE_TEST_PROJECT_COMPLEX_STRUCTS_H
