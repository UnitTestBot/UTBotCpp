int f() {
    struct s {
        int i;
    } *p = 0, *q;
    int j = 0;
    again:
    q = p, p = &((struct s) { j++ });
    if (j < 2) goto again;
    return p == q && q->i == 1; // always returns 1
}
