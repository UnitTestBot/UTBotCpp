/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "simple_loop_uncovered_dot_c_test.h"

#include "gtest/gtest.h"

namespace UTBot {
    TEST(regression, simple_loop_uncovered_test_1) {

        // Construct input
        unsigned int n = 0;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = simple_loop_uncovered(n);

        // Check results
        EXPECT_EQ(expected, actual);
    }
    TEST(regression, simple_loop_uncovered_test_2) {

        // Construct input
        unsigned int n = 255;

        // Expected output
        int expected = 1;

        // Trigger the function
        int actual = simple_loop_uncovered(n);

        // Check results
        EXPECT_EQ(expected, actual);
    }
}

