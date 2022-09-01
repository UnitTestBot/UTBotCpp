#include "bitfields.h"
#include <cassert>
#include <limits>
#include <cstdio>

static int ALIGN = -30;

#define print_sizeof(S) printf("size of %*s : %zu bytes\n", ALIGN, #S, sizeof((S){}))

void print_sizeof_structs() {
    print_sizeof(CppSpecificStruct);
}

CppSpecificStruct modify_and_return_copy(CppSpecificStruct& s) {
    assert(s.b1 >= 0 && s.b1 <= 127);
    assert(s.breaking >= std::numeric_limits<long long>::min());
    assert(s.breaking <= std::numeric_limits<long long>::max());
    assert(s.b2 >= -65536 && s.b2 <= 65535);
    assert(s.b4 >= -2097152 && s.b4 <= 2097151);
    assert(s.b5 >= std::numeric_limits<int>::min());
    assert(s.b5 <= std::numeric_limits<int>::max());
    assert(s.b6 >= -8 && s.b6 <= 7);
    s.b1 = s.b1 * 2 + 1;
    s.breaking = std::numeric_limits<long long>::max();
    if (s.b2 != -65536) {
        s.b2 = -s.b2;
    }
    s.b3 ^= 1;
    if (s.b5 > 0) {
        s.b5 = 10;
    } else {
        s.b5 = -10;
    }
    s.b6 = (s.b1 > 0) + (s.b2 > 0) + (s.b3 > 0) + (s.b4 > 0) + (s.b5 > 0) + (s.b6 > 0);
    return CppSpecificStruct(s);
}
