/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "keywords.h"

#include <stdlib.h>

extern int get_size_of_data(struct data d) {
    return sizeof(d);
}

_Noreturn void stop_now(int i) {
    if (i > 0)
        exit(i);
}

// function's name is C++ operator
static int and (int x, int y) {
    return x & y;
}

static int using(int x) {
    return ++x;
}

// argument's name is C++ keyword
static _Bool different(char *old, char *new) {
    if (old != new) {
        return 1;
    }
    return 0;
}

// function pointer's name is C++ keyword
unsigned char *not_null(void (*catch)(int), unsigned char *x) {
    if (x == NULL) {
        (*catch)(0);
    }
    return x;
}

// record's name is C++ keyword
struct class {
    _Bool flag;
};

_Bool get_flag(struct class clazz) {
    return clazz.flag;
}

// typedef's name is C++ keyword
typedef void *template;

unsigned char *cast(template x) {
    return (unsigned char *)x;
}

// field's name is C++ keyword
struct key {
    long public;
    long private;
};

_Bool equals(struct key key) {
    return key.private == key.public;
}

// enum's value is C++ keyword
enum access { private, protected, public };

int access_to_int(enum access access) {
    if (access == private) {
        return 0;
    }
    if (access == protected) {
        return 1;
    }
    if (access == public) {
        return 2;
    }
    return -1;
}