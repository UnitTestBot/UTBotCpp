#include "inner_basic_functions.h"

int sum_up_to(unsigned int n) {
    int i = 1;
    int sum = 0;
    while (i <= n) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}

int median(int a, int b, int c) {
    int x = a - b;
    int y = b - c;
    int z = a - c;
    if (x * y > 0) {
        return b;
    }
    if (x * z > 0) {
        return c;
    }
    return a;
}
