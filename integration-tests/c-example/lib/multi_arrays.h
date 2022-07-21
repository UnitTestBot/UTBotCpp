#ifndef C_EXAMPLE_MULTI_ARRAYS_H
#define C_EXAMPLE_MULTI_ARRAYS_H

#include <stddef.h>

struct MyStructMult {
    int a[2][3][2];
};

struct IntArray {
    int ints[2][5];
};

struct PointStruct {
    int x;
    int y;
};

int get_elem(int a[1][2]);

int sum_sign(int a[2][2]);

int sum_sign_1d_small(const int a[9]);

int value(int a[2][3]);

int value2(int (*a)[3]);

int value3(int a[]);

int some_method(int **pointer2d);

int return_sign_sum(struct MyStructMult st);

long long return_sign_sum_of_struct_array(struct PointStruct arr[2][2]);

int point_quart(struct PointStruct **point);

struct IntArray return_struct_with_2d_array(int a);

int count_dashes();

#endif //C_EXAMPLE_MULTI_ARRAYS_H
