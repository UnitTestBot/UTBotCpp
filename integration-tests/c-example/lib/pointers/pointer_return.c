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

void *void_pointer_return_int_usage(int a, int b) {
    int *return_val = (int*)malloc(sizeof(int));
    if (a > b) {
        *return_val = a;
    } else {
        *return_val = b;
    }
    return return_val;
}

void *void_pointer_return_char_usage(char *a) {
    return a;
}

int ** returns_2d_pointer_for_int(int a) {
    int **arr = (int **)malloc(4 * sizeof(int *)); 
    for (int i = 0; i < 4; i++) {
        arr[i] = (int *)malloc(4 * sizeof(int)); 
    }
       
    if (a > 0) {
        arr[0][0] = 22;
    } else {
        arr[0][0] = -32;
    }
    return arr;
}

const struct MinMax ** returns_2d_pointer_for_struct(int a, int b) {
    struct MinMax  **arr = (struct MinMax **)malloc(1 * sizeof(struct MinMax *)); 
    arr[0] = (struct MinMax *)malloc(1 * sizeof(struct MinMax)); 
    
    if (a < b) {
        struct MinMax res = {a, b};
        arr[0][0] = res;
    } else {
        struct MinMax res = {b, a};
        arr[0][0] = res;
    }
    return arr;
}

void * return_array_like_void_ptr() {
    void * ptr = malloc(10);
    ((char*)ptr)[0] = 0;
    return ptr;
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
