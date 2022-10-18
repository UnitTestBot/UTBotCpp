#include "floats_special.h"
#include "math.h"

int is_nanf(float x) {
    if (x != x) {
        return 1;
    } else {
        return 0;
    }
}

int is_nan(long double x) {
    if (x != x) {
        return 1;
    } else {
        return 0;
    }

}

int is_inf(float x) {
    if (isinf(x)) {
        return 1;
    } else {
        return 0;
    }
}
