namespace UTBot {
#define _Alignas(x)
#define _Atomic(x) x
#define _Bool bool
#define _Noreturn
#define _Thread_local thread_local

    extern "C" int min(int a, int b);
    struct StaticStruct {
        int x;
        int y;
    };


#define compare compare_func_c
    extern "C" bool compare(int a, int b);
}
#include <cstring>

template <typename T, size_t N>
T from_bytes(const char (&bytes)[N]) {
    T result;
    std::memcpy(&result, bytes, sizeof(result));
    return result;
}
