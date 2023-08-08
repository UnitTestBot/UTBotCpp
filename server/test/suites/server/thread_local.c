#include "thread_local.h"

__thread int thread_local_id;

void set_id(int id) {
    thread_local_id = id + 5;
}

__thread char thread_local_bytes[8];
void set_bytes(char bytes[8]) {
    for (int i = 0; i < 8; ++i) {
        thread_local_bytes[i] = bytes[7 - i];
    }
}

__thread struct ThreadLocalStorage thread_local_storage;
void set_storage(struct ThreadLocalStorage storage) {
    thread_local_storage.fld = - storage.fld - 10;
}