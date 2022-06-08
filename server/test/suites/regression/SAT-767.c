struct String {
    char c[16];
};

struct String** global;

char first() {
   return global[0][0].c[0];
}
