#include "bar.h"
#include "../calc/sum.h"

int f(int a, int b) {
    if (sum(a, b) == a + b) {
        return 1;
    } else {
        return 2;
    }
}
