#include "simple_loop_uncovered.h"

int sum_loops(unsigned int n) {
    return simple_loop_uncovered(1) + simple_loop_uncovered(2);
}
