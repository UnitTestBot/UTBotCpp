/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

typedef unsigned int uint32;

int pg_popcount32_asm(uint32 word) {
    uint32 res;

    __asm__ __volatile__(" popcntl %1,%0\n" : "=q"(res) : "rm"(word) : "cc");
    return (int)res;
}