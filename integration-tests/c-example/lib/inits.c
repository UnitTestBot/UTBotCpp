int a;

void _init_vals() {
    a = 42;
}

int init_global() {
    if(a == 42) {
        return 42;
    }
    return 0;
}
