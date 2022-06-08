#ifndef SIMPLE_TEST_PROJECT_FUNCTION_POINTERS_H
#define SIMPLE_TEST_PROJECT_FUNCTION_POINTERS_H

#include "../structures/structs/simple_structs.h"

int worker(int a, int b);

int receiver(int (*f)(int, int), char c);

void entry_point_func();

char* pointerParam(char* (*f)(int*), int* x);

char pointerToPointer(int (**f)(int), char x);

int structParam(int (*f)(struct MyStruct), const char* s);

int structPointerParam(int (*f)(struct MyStruct*), const int* arr);

int f_add(int a, int b);
int f_sub(int a, int b);
int f_mul(int a, int b);
typedef int (*op_func)(int, int);
typedef int (**op_func_arr)(int, int);
op_func return_op(char op);

int f_chain(op_func_arr functions, int a);

op_func_arr get_chain(char c[]);

struct FStruct {
    int a;
    op_func f;
};

int calcFunctionStruct(struct FStruct function_struct, int b);

// void swap(int* v, int a, int b);
// void my_qsort(int* v, unsigned int left, unsigned int right, int (*comp)(int, int));

#endif //SIMPLE_TEST_PROJECT_FUNCTION_POINTERS_H
