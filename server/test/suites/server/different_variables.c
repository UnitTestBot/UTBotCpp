#include "different_variables.h"

void swap_two_int_pointers(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

float max_of_two_float(float a, float b) {
    if (a > b)
        return a;
    return b;
}

int struct_test(struct easy_str a, struct easy_str b) {
    if (a.a == b.a) {
        return 1;
    }
    return -1;
}
