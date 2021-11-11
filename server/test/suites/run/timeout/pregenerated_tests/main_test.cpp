/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "gtest/gtest.h"

#include <chrono>
#include <thread>

namespace UTBot {
    TEST(regression, two_minutes_test) {
        std::this_thread::sleep_for(std::chrono::minutes(2));
    }
}