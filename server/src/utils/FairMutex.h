#ifndef UNITTESTBOT_FAIRMUTEX_H
#define UNITTESTBOT_FAIRMUTEX_H

#include <condition_variable>
#include <mutex>

// Inspired by https://github.com/yohhoy/yamc/blob/master/include/fair_mutex.hpp
class FairMutex {
    std::size_t next = 0;
    std::size_t curr = 0;
    std::condition_variable cv;
    std::mutex mutex;

public:
    FairMutex() = default;
    ~FairMutex() = default;

    FairMutex(const FairMutex &) = delete;
    FairMutex &operator=(const FairMutex &) = delete;

    void lock();

    bool try_lock();

    void unlock();
};


#endif // UNITTESTBOT_FAIRMUTEX_H
