/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

static int x = 0;
_Bool externed_global = 0;

static int static_simple(int dx)
{
    if (x > 0)
    {
        return x + dx;
    }
    if (x < 0)
    {
        return -x + dx;
    }
    return 0;
}

struct StaticStruct
{
    int x;
    int y;
};

static int static_accept_local_struct(struct StaticStruct str, int index)
{
    if (index == 0)
    {
        return str.x;
    }
    if (index == 1)
    {
        return str.y;
    }
    return 0;
};

static struct StaticStruct static_return_local_struct(int x, int y)
{
    struct StaticStruct res;
    res.x = x;
    res.y = y;
    return res;
};

static inline int static_inline_sum(int a, int b) {
    return a + b;
}
inline int inline_sum(int a, int b) {
    return a + b;
}