/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_KEYWORDS_H
#define UNITTESTBOT_KEYWORDS_H


struct data {
    char x;
    _Alignas(128) char cacheline[64];
};

typedef _Atomic(int) atomic_int;
typedef _Complex double complex_double;



#endif // UNITTESTBOT_KEYWORDS_H
