#include "halt.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int raise_by_num(int num) {
    return raise(num);
}

int raise_stop(int _) {
    return raise(SIGSTOP);
}

void cleanup() {
    fprintf(stderr, "Normal program termination with cleaning up\n");
}

int call_abort() {
    if (atexit(cleanup)) {
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Going to abort the program\n");
    abort();
}
