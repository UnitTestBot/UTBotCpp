#include <stddef.h>

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

int static_adder() {
    static int sum = 0;
    sum++;
    if (sum == 2) {
        return -1;
    }
    return sum;
}

int static_adder_caller(size_t num) {
    int res = 0;
    for (size_t i = 0; i < num; ++i) {
        res = static_adder();
    }
    if (res == 1) {
        return -10;
    } else if (res == -1) {
        return 30;
    } else if (res == 3) {
        return 20;
    }
    return res;
}
