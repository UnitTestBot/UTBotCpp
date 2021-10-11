/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "helloworld.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t helloworld(uint32_t input)
{
    uint32_t output = input + 1;
    printf("hello world input is %u , output is %u\n", input, output);
    return output;
}

#ifdef __cplusplus
}
#endif