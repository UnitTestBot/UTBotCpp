#ifndef UNITTESTBOT_THREAD_LOCAL_H
#define UNITTESTBOT_THREAD_LOCAL_H

struct ThreadLocalStorage {
    int fld;
};

void set_id(int id);

void set_bytes(char bytes[8]);

#endif // UNITTESTBOT_THREAD_LOCAL_H
