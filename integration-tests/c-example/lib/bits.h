/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UINT64
#define UINT64 unsigned long long
#endif 

#ifndef UINT32
#define UINT32 unsigned int
#endif

#define VOS_ERR 1
#define VOS_OK  0

typedef struct tagFOO_ATTRIBUTE {
    UINT32 offset;
    UINT32 len;
    UINT32 id;
} FOO_ATTRIBUTE;

UINT32 NSE_BITS_CLZ64(UINT64 uiiData);

UINT32 foo1(const FOO_ATTRIBUTE *attrib, UINT32 num, UINT32 id, UINT32 *offset, UINT32 *len);