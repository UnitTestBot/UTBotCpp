#include "Coverage.h"

int Coverage::TestStatusMap::getNumberOfTests() {
    int cnt = 0;
    for (auto const &[fileName, testsStatus] : *this) {
        cnt += testsStatus.size();
    }
    return cnt;
}
