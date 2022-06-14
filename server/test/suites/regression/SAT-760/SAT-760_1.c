#include "SAT-760.h"

static struct hash_table *h;

static void write() {
    h = 0;
}

struct hash_table {
    int val;
};
