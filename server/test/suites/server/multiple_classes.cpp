#include "multiple_classes.h"

int first_class::get1() {
    return 1;
}

int second_class::get2() {
    return 2;
}

int second_class::third_class::get3() {
    return 3;
}
