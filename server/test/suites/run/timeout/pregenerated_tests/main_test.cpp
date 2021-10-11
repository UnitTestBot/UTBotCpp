/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "gtest/gtest.h"

namespace UTBot {
    TEST(regression, endless_loop) {
        while (true) {}
    }
}