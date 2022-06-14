#include "FairMutex.h"

void FairMutex::lock() {
    std::unique_lock lk(mutex);
    const std::size_t request = next++;
    while (request != curr) {
        cv.wait(lk);
    }
}

bool FairMutex::try_lock() {
    std::lock_guard lk(mutex);
    if (next != curr)
        return false;
    ++next;
    return true;
}

void FairMutex::unlock() {
    std::lock_guard lk(mutex);
    ++curr;
    cv.notify_all();
}
