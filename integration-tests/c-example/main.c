#include "src/calc.h"
#include "lib/basic_functions.h"

int main(int argc, char* argv[]) {
    if (argc == 2) {
        int k = atoi(argv[1]);
        if (k == 1) {
            return f(k);
        } else {
            return max_(k, 2);
        }
    } else {
        return 1;
    }
}
