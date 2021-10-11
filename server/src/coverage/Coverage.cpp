/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "Coverage.h"

int Coverage::TestStatusMap::getNumberOfTests() {
    int cnt = 0;
    for (auto const &[fileName, testsStatus] : *this) {
        cnt += testsStatus.size();
    }
    return cnt;
}
