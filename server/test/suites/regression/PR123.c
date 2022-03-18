/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include <stdarg.h>
#include <stdio.h>

typedef int (*libbpf_print_fn_t)(const char *, va_list ap);

libbpf_print_fn_t libbpf_set_print(libbpf_print_fn_t fn) {
    return fn;
}