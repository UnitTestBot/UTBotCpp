int hash(const char *s) {
    int h = 0, i = 0;
    while (s[i] != '\0') {
        h = (h + s[i]);
        i++;
    }
    return h;
}
