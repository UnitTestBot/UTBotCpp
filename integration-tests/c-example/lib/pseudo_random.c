#include <stdlib.h>

int sign_rand() {
    srand(42);
    int x = rand();
    if (x > 42) {
        return 1;
    }
    return 0;
}

int sign_rand_seeded(unsigned seed) {
    srand(seed);
    int x = rand();
    if (x > 42) {
        return 1;
    }
    return 0;
}
