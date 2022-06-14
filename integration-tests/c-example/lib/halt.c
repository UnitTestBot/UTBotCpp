#include "halt.h"

#include "signal.h"

int raise_by_num(int num) {
    return raise(num);
}

int raise_stop(int _) {
    return raise(SIGSTOP);
}
