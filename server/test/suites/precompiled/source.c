#include <stdlib.h>

int g() {
    return 15;
}

int main(int argc, char *argv[]) {
    if (abs(argc) < 5) {
        return 0;
    } else {
        return atoi(argv[0]);
    }
}
