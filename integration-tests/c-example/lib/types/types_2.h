#ifndef SIMPLE_TEST_PROJECT_TYPES_2_H
#define SIMPLE_TEST_PROJECT_TYPES_2_H
#include <stdbool.h>
#include <stddef.h>

#ifndef MY_INT
#define MY_INT int
#endif

size_t foo(MY_INT x);

MY_INT bar(size_t a, MY_INT b);

unsigned int diff(int x); 

#endif //SIMPLE_TEST_PROJECT_TYPES_2_H
