#include "function_pointers.h"
#include <stdlib.h> 

int worker(int a, int b) {
    return a - b;
}

int receiver(int f(int, int), char c) {
    if (c == 'a') {
        return f(2, 3);
    } else if (c == 'b') {
        return f(4, 5) + 8;
    } else {
        return -1;
    }
}

void entry_point_func() {
    receiver(worker, 'a');
}

char* pointerParam(char* (*f)(int*), int* x) {
    if (*x == 1) {
        return f(x);
    } else{
        return f(x + 5);
    }
}

char pointerToPointer(int (**f)(int), char x) {
    if (f[0](5) == 4) {
        return x;
    } else {
        return 'z';
    }
}

int structParam(int (*f)(struct MyStruct), const char* s) {
    if (s[0] == 'z') {
        struct MyStruct res = {0, 1};
        return f(res);
    }
    if (s[0] == 'a') {
        struct MyStruct res = {10, 1};
        return f(res) + 6;
    }
    struct MyStruct res = {0, 0};
    return f(res) * f(res);
}

int structPointerParam(int (*f)(struct MyStruct*), const int* arr) {
    struct MyStruct *my_struct = (struct MyStruct*)malloc(sizeof(struct MyStruct));
    if (arr[0] == 1) {
        return f(my_struct) + 12;
    }
    if (arr[1] == 34) {
        return f(my_struct) + 1;
    }
    return f(my_struct);
}

int f_add(int a, int b) {
    return a + b;
}
int f_sub(int a, int b) {
    return a - b;
}
int f_mul(int a, int b) {
    return a * b;
}

op_func return_op(char op) {
    switch(op) {
        case '+': return f_add;
        case '-': return f_sub;
        case '*': return f_mul;
    }
    return NULL;
}

int f_chain(op_func_arr functions, int a) {
    int result = a;
    for (int i = 0; i < 10; i++) {
        result = functions[i](result, a);
    }
    return result;
}

op_func_arr get_chain(char c[]) {
    op_func_arr chain = malloc(sizeof(char*) * 10);
    for (int i = 0; i < 10; i++) {
        switch(c[i]) {
            case '+':
                chain[i] = f_add;
                break;
            case '-':
                chain[i] = f_sub;
                break;
            case '*':
                chain[i] = f_mul;
                break;
            default:
                chain[i] = f_add;
                break;
        }
    }
    return chain;
}

int calcFunctionStruct(struct FStruct function_struct, int b) {
    if (b != 0) {
        return function_struct.f(function_struct.a, b);
    } else {
        return function_struct.f(function_struct.a, 10);
    }
}

// void swap(int* v, int a, int b) {
//     int tmp = v[a];
//     v[a] = v[b];
//     v[b] = tmp;
// }

// void my_qsort(int* v, unsigned int left, unsigned int right, int (*comp)(int, int)) {
//     unsigned int i, last;

//     if (left >= right) return;
//     swap(v, left, (left + right) / 2);
//     last = left;
//     for (i = left + 1; i <= right; i++) {
//         if ((*comp)(v[i], v[left]) < 0) {
//             swap(v, ++last, i);
//         }
//     }
//     swap(v, left, last);
//     my_qsort(v, left, last-1, comp);
//     my_qsort(v, last+1, right, comp);
// }

typedef struct Return {
  int y;
} Return;
typedef struct Param {
  int x;
} Param;
typedef Return *(*Functor)(Param *);
struct Container {
  Functor f;
};
static int apply(struct Container container, Param *param) {
  struct Return *value = container.f(param);
  if (value->y < 0) {
    return value->y;
  }
  return value->y;
}
static int apply_stub(struct Container container, Param *param) {
  return 0;
}
