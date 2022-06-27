#include "pointer_return.h"
#include <stdlib.h>

long long* returns_pointer_with_min(long long a, long long b) {
    static long long return_val;
    if (a < b) {
        return_val = a;
    } else {
        return_val = b;
    }

    return (&return_val);
}

unsigned int* returns_pointer_with_max(unsigned int a, unsigned int b) {
    unsigned int *return_val = (unsigned int*)malloc(sizeof(unsigned int));
    if (a > b) {
        *return_val = a;
    } else {
        *return_val = b;
    }

    return return_val;
}

int* five_square_numbers(int from) {
    static int sq[5];

    for (int i = 0; i < 5; i++) {
        sq[i] = from * from;
        from++;
    }

    return sq;
}

struct MinMax* returns_struct_with_min_max(int a, int b) {
    struct MinMax *min_max = (struct MinMax*)malloc(sizeof(struct MinMax));
    if (a < b) {
        min_max->a = a;
        min_max->b = b;
    } else {
        min_max->b = a;
        min_max->a = b;
    }

    return min_max;
}

const char* return_const_char(int a) {
    if (a == 0) {
        return "aaaa";
    }
    return "bbbb";
}

char const * return_char_const_pointer(int a) {
    if (a == 0) {
        return "a";
    } else {
        return "b";
    }
}

const struct MinMax* returns_const_struct_with_min_max(int a, int b) {
    struct MinMax *min_max = (struct MinMax*)malloc(sizeof(struct MinMax));
    if (a < b) {
        min_max->a = a;
        min_max->b = b;
    } else {
        min_max->b = a;
        min_max->a = b;
    }

    return min_max;
}

void *void_pointer_return_char_usage(char *a) {
    return a;
}

int *return_int_array(int* a) {
    a[0] = 5;
    return a;
}

long long* return_long_long_array(int a, int b) {
    long long *array = (long long*)malloc(sizeof(long long) * 10);
    array[5] = a;
    array[0] = b;
    return array;
}

static char memory[3][3];
void ** return_void_2d() {
    return (void **)(&memory[0][0]);
}

int* return_nullptr(int x) {
    static int ret = 5;
    static int nine = 9;
    if (x == 11) {
        return &nine;
    } else if (x == 0) {
        return NULL;
    } else {
        return &ret;
    }
}

const struct SupportedStruct4* return_null_struct(int x) {
    const struct SupportedStruct4 *res;
    if (x == 0) {
        return NULL;
    }
    res = (struct SupportedStruct4*)malloc(sizeof(const struct SupportedStruct4));
    return res;
}
