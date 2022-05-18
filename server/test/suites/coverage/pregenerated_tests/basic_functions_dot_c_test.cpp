/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "basic_functions_dot_c_test.h"

#include "gtest/gtest.h"

namespace UTBot {
    TEST(regression, max__test_1) {

        // Construct input
        int a = 0;
        int b = 0;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = max_(a, b);

        // Check results
        EXPECT_EQ(expected, actual);
    }
    TEST(regression, max__test_2) {

        // Construct input
        int a = 0;
        int b = -1;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = max_(a, b);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, min_test_1) {

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
    TEST(regression, min_test_2) {

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

    TEST(regression, sqr_positive_test_1) {

        // Construct input
        int a = 0;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = sqr_positive(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }
    TEST(regression, sqr_positive_test_2) {

        // Construct input
        int a = -1;

        // Expected output
        int expected = -1;

        // Trigger the function
        int actual = sqr_positive(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }
}
