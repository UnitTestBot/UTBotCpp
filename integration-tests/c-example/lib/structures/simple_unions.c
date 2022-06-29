#include "simple_unions.h"
#include <stddef.h>

int get_sign_union(union IntBytesUnion st) {
    if (st.number == 0) {
        return 0;
    }
    if (st.number < 0) {
        return -1;
    } else {
        return 1;
    }
}

int extract_bit(union ShortBytesUnion x) {
    if (x.bytes[0] > 0) {
        return 0;
    }
    if (x.bytes[1] > 0) {
        return 1;
    }
    return -1;
}


int calculate_something_union(union Heterogeneous str) {
    if (str.a == 1 || str.b == (1LL << 20)) {
        return 0;
    }
    if (str.a == 7 || str.b == 2020) {
        return 1;
    }
    if ((long long) str.a > str.b) {
        return str.a;
    } else {
        return (int) str.b;
    }
}

int RawMantissa(union SuperFloat f) {
    return f.i & ((1 << 23) - 1);
}

int get_coordinate(union Vector2D vect, int i) {
    // assert(i >= 0 && i < 2);
    if (i >= 0 && i < 2) {
        return vect.m[i];
    }
    return 0;
}

signed char operate_with_inner_unions(union MainUnion st) {
    if (st.x == 5 || st.y == 5 || st.inner.c == '5' ||
        st.inner.s == 5 || st.inner.ininner.l == 5 || st.inner.ininner.u == 5) {
        return '5';
    }

    if (st.x == 5 || st.y == 102 || st.inner.s == 15) {
        return st.inner.c;
    }

    if ((long long) st.inner.ininner.u == st.inner.ininner.l) {
        return 'e';
    }
    if ((long long) st.inner.ininner.u > st.inner.ininner.l) {
        return 'g';
    }

    return 'o';
}


union MainUnion union_as_return_type(int a) {
    if (a == 0) {
        union MainUnion res = {{.c='0'}};
        return res;
    }

    if (a == 1) {
        union MainUnion res = {{.ininner={.l=1}}};
        return res;
    }

    union MainUnion res = {.x=2};
    return res;
}

// SAT-303
// int to_int(struct Variant variant)
// {
//     switch (variant.tag)
//     {
//         case CHAR_TAG:
//             return (int) variant.data.c;
//         case INT_TAG:
//             return variant.data.i;
//         default:
//             return 0;
//     }
// }

int sumOfUnionArray(union IntBytesUnion u[5]) {
    int res = 0;
    for (int i = 0; i < 5; ++i) {
        res += u[i].number;
    }
    return res;
}

long long operateWithUnionWithPointer(union UnionWithPointer un) {
    if (un.b == 1) {
        return 1;
    }
    
    if (un.a == NULL) {
        return 0;
    }
    return -1;
}
