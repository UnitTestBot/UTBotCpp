#include "loops.h"
#include <stdbool.h>

unsigned int while_loop(unsigned int n) {
    unsigned int i = 0;
    while (i < n) {
        i = i + 1;
        if (n % i == 37U)
            return 1;
        else if (i == 50U)
            return 2U;
    }
    return 0U;
}

int do_while_loop(int n) {
    int i = 0;
    do {
        i = i + 1;
        if (n % i == 37)
            return 1;
        else if (i == 50)
            return 2;
    } while (i < n);
    return 0;
}

int continue_break(int n) {
    int i = n, res = 0;
    do {
        res += i;
        if (res > 100) {
            break;
        }
        if (i < 20) {
            continue;
        }
    } while (false);
    return res;
}

int goto_keyword(unsigned int a) {
    unsigned int sum = 0;
    do {
        if (a == 15) {
            goto RET;
        }
        sum += a;
        a++;
    } while (a < 22);
    if (sum > 1000) {
        return 1;
    }
    return 2;
    RET: return -1;
}
