#ifndef SIMPLE_TEST_PROJECT_TYPES_H
#define SIMPLE_TEST_PROJECT_TYPES_H

#include <stdbool.h>
#include <wchar.h>
#include <string.h>

struct SimpleStruct {
    int a;
    char *c;
};

struct LinkedStruct {
    struct LinkedStruct *st;
};

struct SimpleStructWithUnion {
    union U {
        int a;
        char c;
    } u;
};

struct PtrCharStruct {
    char *c;
};

struct ConstFieldStruct {
    short b;
    const int a;
    char c;
};

struct IncompleteType {
    char c[100];
};

char a_or_b(char a, char b);

signed long long max_long(long long a, signed long long b);

unsigned int max_unsigned_signed(unsigned int a, int b);

short min_short(short a, short b);

short int min_divided_by_2(signed short a, signed short int b);

signed char some_func(char a, unsigned char b);

wchar_t wide_char(wchar_t a, wchar_t b);

int fun_that_accept_bools(_Bool a, bool b);

bool is_positive(int arg);

int supported_parameter_1(struct SimpleStruct st);

int supported_parameter_2(struct LinkedStruct st);

const struct PtrCharStruct structWithConstPointerReturn(int a);

int structWithConstPointerParam(struct PtrCharStruct st);

const struct PtrCharStruct *structWithConstPointerReturnPointer(int a);

int structWithConstFields(struct ConstFieldStruct st);

void pointer_to_pointer(int **a);

int void_pointer(void *a);

// This function should be skipped, as structs with unions are not supported.
struct SimpleStructWithUnion structWithUnion(int a);

#endif //SIMPLE_TEST_PROJECT_TYPES_H
