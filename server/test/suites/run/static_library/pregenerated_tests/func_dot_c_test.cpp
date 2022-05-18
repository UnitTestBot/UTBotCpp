/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "func_dot_c_test.h"

#include "gtest/gtest.h"

namespace UTBot {
    TEST(regression, cmp_test_1) {
        // Construct input
        int a = 0;
        int b = 0;

        // Expected output
        _Bool expected = false;

        // Trigger the function
        _Bool actual = compare(a, b);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, min_test_1) {
        // Construct input
        int a = 0;
        int b = 1;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = min(a, b);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, min_test_2) {
        // Construct input
        int a = 0;
        int b = 0;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = min(a, b);

        // Check results
        EXPECT_EQ(expected, actual);
    }
}