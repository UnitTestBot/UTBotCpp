#include "globals.h"

_Bool externed_global = 0;
_Bool externed_array[1] = {0};
struct ExternStruct extern_struct = {
    .fld = 0
};

int ordinary_sum(int a, int b) {
    return a + b;
}

extern int extern_sum(int a, int b) {
    return a + b;
}

static int static_sum(int a, int b) {
    return a + b;
}
