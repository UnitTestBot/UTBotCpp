#include <iostream>

int main() {
    std::cout << "Hello" << "\n";
    int counter = 0;
    for (auto a = 0; a < 100; a++) {
        if (a % 2 == 0) {
            counter += 2;
        }
        else {
            counter++;
        }
    }
    std::cout << counter << "\n";
    return 0;
}