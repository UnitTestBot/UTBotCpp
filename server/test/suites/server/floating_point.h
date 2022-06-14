#ifndef SIMPLE_TEST_PROJECT_FPA_H
#define SIMPLE_TEST_PROJECT_FPA_H
#include "math.h"

struct FParray {
    float data[2];
};

int get_double_sign(double x);

int is_close(float_t x, int y);

float long_double_arith(long double x);

int array_max(float arr []);

struct FParray fp_array(int a);
#endif //SIMPLE_TEST_PROJECT_FPA_H
