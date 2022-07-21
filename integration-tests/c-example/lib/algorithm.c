#include <stdlib.h>

int abs_val(int x) {
    if (x < 0 && abs(x) > 0) {
        return -1;
    } else if (x > 0) {
        return 1;
    }
    return abs(x);
}

int sign(char const *str) {
    int x = atoi(str);
    if (x == 5) {
        return 7;
    }
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    }
    return 0;
}
