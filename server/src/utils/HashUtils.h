#ifndef UNITTESTBOT_HASHUTILS_H
#define UNITTESTBOT_HASHUTILS_H

#include "utils/path/FileSystemPath.h"

namespace tests {
    struct TestMethod;
}

class StubOperator;

namespace HashUtils {
    template <typename T, typename... Rest>
    inline void hashCombine(std::size_t &seed, T const &v, Rest &&...rest) {
        size_t h;
        if constexpr (std::is_same_v<T, fs::path>) {
            h = fs::hash_value(v);
        } else {
            std::hash<T> hasher;
            h = hasher(v);
        }
        seed ^= h + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, std::forward<Rest>(rest)), ...);
    }

    struct PathHash {
        std::size_t operator()(const fs::path &path) const;
    };

    struct StubHash {
        std::size_t operator()(const StubOperator &stub) const;
    };

    struct TestMethodHash {
        std::size_t operator()(const tests::TestMethod &testMethod) const;
    };
}

#endif //UNITTESTBOT_HASHUTILS_H
