#include "assertion_failures.h"

int buggy_function2(int a) {
    assert(a == 7);
    assert(a < 7);
    return a;
}
