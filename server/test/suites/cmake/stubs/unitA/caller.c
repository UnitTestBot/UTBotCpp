#include "callee.h"

int foo(int x, int y) {
    return do_stuff(x, y-x);
}

//int main() {
//    int res = do_stuff(0, 0);
//    return 0;
//}