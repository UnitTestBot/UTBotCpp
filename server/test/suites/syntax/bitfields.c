#include "bitfields.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static int ALIGN = -30;

#define print_sizeof(S) printf("size of %*s : %zu bytes\n", ALIGN, #S, sizeof((S){}))

void print_sizeof_structs() {
    print_sizeof(SimpleSignedStr);
    print_sizeof(SimpleUnsignedStr);
    print_sizeof(ImplementationDefinedStr);
    print_sizeof(PossiblySmallStr);
    print_sizeof(SimpleUnsignedUnion);
    print_sizeof(ComplexStr);
    print_sizeof(StrWithBool);
    print_sizeof(StrWithUnnamedBitfields);
    print_sizeof(StrWithUnnamedZeroBitfield);
    print_sizeof(StrWithBreak);
}

int check_simple_signed_str(SimpleSignedStr s) {
    if (s.a == 1024 && s.b == -1 && s.d == -16) {
        return 1;
    } else if (s.b == 0) {
        return -1;
    }
    return 0;
}

int is_sum_greater_10(SimpleUnsignedStr s) {
    return (s.a + s.b + s.c + s.d > 10 && s.d > 5);
}

int is_all_greater_2(PossiblySmallStr s) {
    if (s.a > 2 && s.b > 2) {
        return 1;
    }
    return 0;
}

int union_greater_2(SimpleUnsignedUnion u) {
    if (u.c > 2) {
        return 1;
    }
    return 0;
}

int union_greater_2_short(SimpleUnsignedUnion u) {
    return u.c > 2;
}

int sum_of_fields(ComplexStr s) {
    if (!s.a) {
        return -1;
    }
    return s.a + s.b + s.c + s.d + s.e;
}

int decode_from_bool(StrWithBool s) {
    if (!s.a && !s.b && !s.c) {
        return 0;
    } else if (s.a && !s.b && !s.c) {
        return 1;
    } else if (!s.a && s.b && !s.c) {
        return 2;
    } else if (s.a && s.b && !s.c) {
        return 3;
    } else if (!s.a && !s.b && s.c) {
        return 4;
    } else if (s.a && !s.b && s.c) {
        return 5;
    } else if (!s.a && s.b && s.c) {
        return 6;
    }
    return 7;
}

int decode_from_bool_simple(StrWithBool s) {
    if (!s.a && !s.b && !s.c) {
        return 0;
    } else if (s.a && !s.b && s.c) {
        return 5;
    }
    return 7;
}

 StrWithUnnamedBitfields mult_by_two(StrWithUnnamedBitfields s) {
     s.b1 *= 2;
     s.b2 *= 2;
     s.b3 *= 2;
     return s;
 }

 int is_nice(StrWithUnnamedZeroBitfield s) {
     if (s.b1 == 69 && s.b2 == 42 && s.b3 == 1488) {
         return 13;
     }
     return 0;
 }

int check_fields_bounds(StrWithBreak s) {
    assert(s.b1 >= 0 && s.b1 <= 127);
    assert(s.breaking >= LLONG_MIN && s.breaking <= LLONG_MAX);
    assert(s.b2 >= -65536 && s.b2 <= 65535);
    assert(s.b3 == true || s.b3 == false);
    assert(s.b4 >= -2097152 && s.b4 <= 2097151);
    if (s.b1 >= 123 && s.b3) {
        return 1;
    } else if (s.b1 >= 123 && s.b4 < 0) {
        return 2;
    } else if (s.breaking > 42) {
        return 3;
    }
    return 4;
}

void simple_modify(SimpleSignedStr* s) {
    s->a++;
    s->b = ~s->b;
    if (s->c >= 0) {
        s->c *= 2;
    }
    s->d /= 2;
}

SimpleSignedStr* create_on_heap(int a, int b, int c, int d) {
    SimpleSignedStr* s = malloc(sizeof(SimpleSignedStr));
    if (s) {
        s->a = s->b = s->c = s->d = -1;
        if (a >= -8388608 && a <= 8388607) {
            s->a = a;
        }
        if (b >= -1 && b <= 0) {
            s->b = b;
        }
        if (c >= -2 && c <= 1) {
            s->c = c;
        }
        if (d >= -16 && d <= 15) {
            s->d = d;
        }
    }
    return s;
}
