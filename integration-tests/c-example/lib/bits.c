/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "bits.h"

UINT32 NSE_BITS_CLZ64(UINT64 uiiData)
{
    UINT64 uiiMask;
    UINT64 i;
    UINT32 zeroCnt;

    zeroCnt = 0;
    uiiMask = 0x8000000000000000U;
    for (i = 0; i < 64; i++) {
        if ((uiiData & uiiMask) == 0) {
            zeroCnt++;
            uiiMask >>= 1;
            continue;
        }
        break;
    }

    return zeroCnt;
}

UINT32 foo1(const FOO_ATTRIBUTE *attrib, UINT32 num, UINT32 id, UINT32 *offset, UINT32 *len)
{
    UINT32 i;

    for (i = 0; i < num; i++) {
        if (id == attrib[i].id) {
            *offset = attrib[i].offset;
            *len = attrib[i].len;
            return VOS_OK;
        }
    }

    return VOS_ERR;
}