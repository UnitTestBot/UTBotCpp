/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

int counter = 0;

int increment()
{
    ++counter;
    return counter;
}

void *memory = 0;
void **memory2 = 0;
static _Bool static_global = 0;
static const _Bool const_global = 0;
_Bool non_static_global = 0;
extern _Bool externed_global;
extern _Bool externed_global;
extern int externed_int_no_def;

static int use_globals(int default_value)
{
    if (memory) {
        return *(int *)memory;
    }
    if (memory2 && *memory2) {
        return **(int**)memory2;
    }
    if (static_global)
    {
        return 1;
    }
    if (const_global)
    {
        return 2;
    }
    if (non_static_global)
    {
        return 3;
    }
    if (externed_global) {
        return 4;
    }
    return default_value;
}

int const global_const_array[3] = {1, 2, 4};
int global_array[3] = {1, 2, 3};
int use_global_array(int x) {
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += global_array[i] * global_const_array[i];
    }
    if (sum > 0) {
        return sum;
    }
    if (sum < 0) {
        return -sum;
    }
    return 0;
}

static char* global_mutable_string;
static const char* global_const_string;

char use_global_strings() {
    if (!global_mutable_string) {
        return 'M';
    }
    if (!global_const_string) {
        return 'C';
    }
    char c = global_const_string[0];
    char res;
    if (c >= 'a' && c <= 'z') {
        res = 'A' + c - 'a';
    } else {
        res = c;
    }
    global_mutable_string[0] = res;
    return res;
}

static int* global_mutable_int_array;
static const int* global_const_int_array;

int use_global_arrays() {
    if (!global_mutable_int_array) {
        return -1;
    }
    if (!global_const_int_array) {
        return -1;
    }
    int c = global_const_int_array[0];
    int res;
    if (c < 0) {
        res = -c;
    } else {
        res = +c;
    }
    global_mutable_int_array[0] = res;
    return res;
}

typedef void (*handler_type)(int);
handler_type handler;

void use_global_handler(int status) {
    if (status == 0) {
        return;
    }
    handler(status);
}