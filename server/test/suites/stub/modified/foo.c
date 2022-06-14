#include "foo.h"
#include "sum.h"
#include "mult.h"

int check_stubs(int a, int b) {
    if (sum(a, b, 1) == 69) {
        return 1;
    }
    if (mult(a, b) == 322) {
        return 2;
    }
    if (mult(a, b) - sum(a, b, 1) == 228) {
        return 3;
    }
    if (sum(a, b, 0) == a + b) {
        return 4;
    } else {
        return 5;
    }
}
