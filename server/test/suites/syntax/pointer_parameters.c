#include "pointer_parameters.h"

int c_strcmp(const char *a, const char *b) {
    for (int i = 0;; i++) {
        if (a[i] != b[i]) {
            return 0;
        } else {
            if (a[i] == '\0' || b[i] == '\0') {
                return a[i] == '\0' && b[i] == '\0';
            }
        }
    }
}

int ishello(const char *a) {
    return c_strcmp(a, "hello");
}

int longptr_cmp(long *a, long *b) {
    return (*a == *b);
}

int void_pointer_int_usage(void *x) {
    int *a = x;
    return a[0];
}

int pointer_as_array_parameter(int *a, int *b, int c) {
    a[1] = 3;
    *b = 4;
    return a[1] + *b + c;
}

typedef int int_array[1];

int typedef_as_parameter_and_return(int_array* a) {
    if((*a)[0] == 0) {
        (*a)[0] = 42;
    } else {
        (*a)[0] = 24;
    }
    return (*a)[0];
};

int void_ptr(int a, int b, void *ptr) {
    if (a + b > 0) {
        return 1;
    } else if (a + b < -5) {
        return -1;
    } else {
        return 0;
    }
}
