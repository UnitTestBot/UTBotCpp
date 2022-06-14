#ifndef SIMPLE_TEST_PROJECT_TYPES_H
#define SIMPLE_TEST_PROJECT_TYPES_H
#include <stdbool.h>

char a_or_b(char a, char b);

signed long long max_long(long long a, signed long long b);

short min_short(short a, short b);

short int min_divided_by_2(signed short a, signed short int b);

signed char some_func(char a, unsigned char b);

int fun_that_accept_bools(_Bool a, bool b);

bool is_positive(int arg);
#endif //SIMPLE_TEST_PROJECT_OTHER_TYPES_h
