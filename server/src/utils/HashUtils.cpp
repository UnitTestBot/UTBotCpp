#include "HashUtils.h"

#include "Tests.h"

#include "Synchronizer.h"

namespace HashUtils {
    std::size_t PathHash::operator()(const fs::path &path) const {
        return fs::hash_value(path);
    }

    std::size_t StubHash::operator()(const StubOperator &stub) const {
        size_t seed = 0;
        hashCombine(seed, stub.getSourceFilePath(), stub.isHeader());
        return seed;
    }

    std::size_t TestMethodHash::operator()(const tests::TestMethod &testMethod) const {
        size_t seed = 0;
        hashCombine(seed, testMethod.methodName,
                    testMethod.bitcodeFilePath,
                    testMethod.sourceFilePath,
                    testMethod.is32bits);
        return seed;
    }
}
