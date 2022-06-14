#include "func.h"

static _Bool compare(int a, int b) {
    return a < b;
}

int min(int a, int b) {
    if (compare(a, b)) {
        return a;
    } else {
        return b;
    }
}
