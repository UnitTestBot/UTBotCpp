#include "stdlib.h"
#include "string.h"

int out_of_bound_access_to_heap(int num) {
    int *p = calloc(5, sizeof(int));
    return p[num];
}

int out_of_bound_access_to_stack(int num) {
    int a[5];
    memset(a, 0, sizeof(a));
    return a[num];
}

int g[5];

int out_of_bound_access_to_globals(int i) {
    return g[i];
}

int use_after_free() {
    int *p = calloc(5, sizeof(int));
    free(p);
    return *p;
}

static char* passthrough(char *p) {
    return p;
} 

static char* leak_stack() {
    char x[524];
    memset(x, 0, sizeof(x));
    return passthrough(&x[0]);
}

char use_after_return() {
    char x = *leak_stack();
    return x;
}

int use_after_scope() {
    int *p = 0;
    {
        int x = 0;
        p = &x;
    }
    return *p;
}

void double_free() {
    int *p = calloc(5, sizeof(int));
    free(p);
    free(p);
}

int invalid_free() {
    char p[524];
    free(p);
    return p[0];
}
