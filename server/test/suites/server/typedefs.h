/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef SIMPLE_TEST_PROJECT_CHECK_TYPEDEF_H
#define SIMPLE_TEST_PROJECT_CHECK_TYPEDEF_H
#include <sys/types.h>
#include <stddef.h>

int gid_to_name (gid_t gid);

typedef struct {
    int a;
} TypeDefStruct;

typedef struct __typeDefStruct {
    int a;
} TypeDefStruct2;

typedef size_t size_t_alias;

int sign_of_typedef_struct(TypeDefStruct x);

int sign_of_typedef_struct2(TypeDefStruct2 x);

size_t min_size_t(size_t a, size_t b);

size_t_alias min_size_t_alias(size_t_alias a, size_t_alias b);

#endif