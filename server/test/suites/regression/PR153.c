#include <assert.h>

typedef int (*comparison_function)(void const *, void const *);
void unused(comparison_function cmp) {
    assert(0);
}
