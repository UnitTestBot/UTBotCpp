#include "Coverage.h"

int Coverage::TestResultMap::getNumberOfTests() {
    int cnt = 0;
    for (auto const &[fileName, testsResult] : *this) {
        cnt += testsResult.size();
    }
    return cnt;
}
