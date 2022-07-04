#include "test.h"

int test(int x) {
    if (sum(x, 2 * x) > 5) {
        return 1;
    } else {
        return -1;
    }
}
