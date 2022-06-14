#include "floating_point_plain.h"

int plain_isnan(float x) {
    if (x != x) {
        return 1;
    } else {
        return 0;
    }
}
