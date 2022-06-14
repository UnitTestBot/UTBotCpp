#ifndef SIMPLE_TEST_PROJECT_TYPES_H
#define SIMPLE_TEST_PROJECT_TYPES_H
#include <stdbool.h>

struct SupportedStruct1 {
    int a;
    char * c;
};

struct SupportedStruct2 {
    struct SupportedStruct2* st;
};

struct UnsupportedStruct3 {
    union U {
        int a;
        char c;
    } u;
};

struct SupportedStruct4 {
    char* c;
};

struct SupportedStruct5 {
    short b;
    const int a;
    char c;
};

char a_or_b(char a, char b);

signed long long max_long(long long a, signed long long b);

short min_short(short a, short b);

short int min_divided_by_2(signed short a, signed short int b);

signed char some_func(char a, unsigned char b);

int fun_that_accept_bools(_Bool a, bool b);

bool is_positive(int arg);

int supported_parameter_1(struct SupportedStruct1 st);

int supported_parameter_2(struct SupportedStruct2 st);

const struct SupportedStruct4 structWithConstPointerReturn(int a);

int structWithConstPointerParam(struct SupportedStruct4 st);

const struct SupportedStruct4* structWithConstPointerReturnPointer(int a);

int structWithConstFields(struct SupportedStruct5 st);

void supported_pointer(int ** a);

int void_pointer(void * a);

// This function should be skipped, as structs with unions are not supported.
struct UnsupportedStruct3 structWithUnion(int a);

#endif //SIMPLE_TEST_PROJECT_OTHER_TYPES_h
