_Bool externed_global = 0;

int ordinary_sum(int a, int b) {
    return a + b;
}

extern int extern_sum(int a, int b) {
    return a + b;
}

static int static_sum(int a, int b) {
    return a + b;
}
