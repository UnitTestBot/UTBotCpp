/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "BuildResult.h"

BuildResult::Type operator|(BuildResult::Type a, BuildResult::Type b) {
    if (a == BuildResult::Type::NONE) {
        return b;
    }
    if (b == BuildResult::Type::NONE) {
        return a;
    }
    if (a == BuildResult::Type::NO_STUBS && b == BuildResult::Type::NO_STUBS) {
        return BuildResult::Type::NO_STUBS;
    }
    if (a == BuildResult::Type::ALL_STUBS && b == BuildResult::Type::ALL_STUBS) {
        return BuildResult::Type::ALL_STUBS;
    }
    return BuildResult::Type::ANY_STUBS;
}

BuildResult::Type operator|=(BuildResult::Type &a, BuildResult::Type b) {
    return a = a | b;
}
