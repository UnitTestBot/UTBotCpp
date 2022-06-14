#include "functions_as_params.h"
#include <stdlib.h>

int receiver(int f(int, int), char c) {
    if (c == 'a') {
        return f(2, 3);
    } else if (c == 'b') {
        return f(4, 5) + 8;
    } else {
        return -1;
    }
}

char* pointerParam(char* (*f)(int*), int* x) {
    if (*x == 1) {
        return f(x);
    } else{
        return f(x + 5);
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

int calcFunctionStruct(struct FStruct function_struct, int b) {
    if (b != 0) {
        return function_struct.f(function_struct.a, b);
    } else {
        return function_struct.f(function_struct.a, 10);
    }
}

// inspired by postgres
typedef struct Return {
    int y;
} Return;
typedef struct Param {
    int x;
} Param;
typedef Return* (*Functor)(Param*);
struct Container {
    Functor f;
};
static int apply(struct Container container, Param *param) {
    struct Return* value = container.f(param);
    if (value->y < 0) {
        return value->y;
    }
    return value->y;
}
