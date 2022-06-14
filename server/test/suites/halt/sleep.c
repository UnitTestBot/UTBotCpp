#include <unistd.h>

int sleepy(int x) {
    if (x > 0) {
        sleep(50);
        if (x == 0) {
            return 0;
        }
        return -1;
    } else {
        return 1;
    }
}
