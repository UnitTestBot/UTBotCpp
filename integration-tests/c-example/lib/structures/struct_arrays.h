#ifndef SIMPLE_TEST_PROJECT_ARRAYS_H
#define SIMPLE_TEST_PROJECT_ARRAYS_H

struct CharAndInt {
    char c;
    int x;
};

struct Trio {
    int a;
    long long b;
    short c;
};

int index_of_needed_struct(struct CharAndInt *arr);

int index_of_struct_with_equal_fields(struct Trio arr []);

#endif //SIMPLE_TEST_PROJECT_DEPENDED_FUNCTIONS_H
