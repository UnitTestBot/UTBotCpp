#include "pair.h"

struct pair {
    int x, y;
};

int sum(struct pair *p) {
    if (p) {
        return p->x + p->y;
    }
    return 0;
}
