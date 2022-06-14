#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (f(argc) < 5) {
        return 0;
    } else {
        return atoi(argv[0]);
    }
}
