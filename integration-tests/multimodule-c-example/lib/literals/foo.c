#include "foo.h"
#include "sum.h"
#include "mult.h"

int check_niceness(int a, int b) {
    if (sum(a, b) == 69) {
        return 1;
    }
    if (mult(a, b) == 322) {
        return 2;
    }
    if (mult(a, b) - sum(a, b) == 228) {
        return 3;
    }
    return 4;
}
