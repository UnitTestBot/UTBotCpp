#include "types.h"
#include <stdio.h>
#include <stdlib.h>

char a_or_b(char a, char b) {
    if (a == 'a') {
        return a;
    }
    if (b == 'b') {
        return b;
    }

    if (a > b) {
        return a;
    }
    return b;
}

signed long long int max_long(long long a, signed long long b) {
    if (a > b) {
        return a;
    }
    return b;
}

short min_short(short a, short b) {
    if (a < b) {
        return a;
    }

    return b;
}

short int min_divided_by_2(signed short a, signed short int b) {
    return min_short(a, b) / 2;
}

signed char some_func(char a, unsigned char b) {
    if (b == 'z' && a > b) return a;
    if (b != 'z') return b;
    return '0';
}

int fun_that_accept_bools(_Bool a, bool b) {
    if (a && b) return 1;
    if (a) return 2;
    if (b) return 3;
    return 4;
}

bool is_positive(int arg) {
    if (arg > 0) return true;
    return false;
}


int supported_parameter_1(struct SupportedStruct1 st) {
    return 1;
}

int supported_parameter_2(struct SupportedStruct2 st) {
    return 2;
}


void supported_pointer(int ** a) {
    if (a[0][0] == 1) {
        a[1][1] = 2;
    }
}

int void_pointer(void * a) {
    return 2;
}

_Bool bool_array_head(_Bool* b) {
    return b[0];
}

const struct SupportedStruct4 structWithConstPointerReturn(int a) {
    if (a % 2 == 0) {
        struct SupportedStruct4 res = {.c = "abcd"};
        return res;
    }
    struct SupportedStruct4 res = {.c = "ABCD"};
    return res;
}

struct UnsupportedStruct3 structWithUnion(int a) {
    if (a < 0) {
        struct UnsupportedStruct3 res;
        res.u.a = a;
        return res;
    } else {
        struct UnsupportedStruct3 res;
        res.u.c = '2';
        return res;
    }
}

const struct SupportedStruct4* structWithConstPointerReturnPointer(int a) {
    const struct SupportedStruct4 *res = (struct SupportedStruct4*)malloc(sizeof(const struct SupportedStruct4));
    return res;
}

int structWithConstPointerParam(struct SupportedStruct4 st) {
    if (st.c == NULL) {
        return 0;
    }

    return 2;
}

struct IncompleteType {
    char c[100];
};
