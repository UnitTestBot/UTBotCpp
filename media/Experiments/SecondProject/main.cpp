#include <iostream>
#include "SimpleClass.h"

void foo(int a) {
    int sum = 0;
    for (auto i = 1; i <= a; i++) {
        sum += i;
    }
    auto my = new SimpleClass(sum);
    my->count *= 5;
}

int main() {
    std::cout << "Hello" << "\n";
    int a = 5;
    for (auto k = 0; k < 2000000; k++) {
        if (k % 1893 == 0) {
            foo(k);
        }
        else {
            a++;
        }
    }
    for (auto k = 0; k < 20; k++) {
        std::cout << "hello";
    }
    std::cout << "\n";
    return 0;
}