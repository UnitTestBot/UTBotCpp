#include "methods_with_asserts_dot_cpp_test.h"

#include "gtest/gtest.h"

#include "access_private.hpp"


namespace UTBot {
    static const float utbot_abs_error = 1e-6;


#pragma region regression
    TEST(regression, first_function_test_1)
    {
        // Construct input
        int a = 5;



        // Expected output
        int expected = 5;

        // Trigger the function
        int actual = first_function(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, second_function_test_1)
    {
        // Construct input
        int a = 0;



        // Expected output
        int expected = 5;

        // Trigger the function
        int actual = second_function(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, get_sign_test_1)
    {
        // Construct input
        int a = -2;



        // Expected output
        int expected = 5;

        // Trigger the function
        int actual = get_sign(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, get_sign_test_2)
    {
        // Construct input
        int a = 1;



        // Expected output
        int expected = 5;

        // Trigger the function
        int actual = get_sign(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, fourth_function_test_1)
    {
        // Construct input
        int a = 0;



        // Expected output
        int expected = 5;

        // Trigger the function
        int actual = fourth_function(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }

    TEST(regression, fourth_function_test_2)
    {
        // Construct input
        int a = 1;



        // Expected output
        int expected = 5;

        // Trigger the function
        int actual = fourth_function(a);

        // Check results
        EXPECT_EQ(expected, actual);
    }

#pragma endregion
#pragma region error
    TEST(error, first_function_test_2)
    {
        // Construct input
        int a = 0;



        /*Error: ASSERTION FAIL: a > 5
        File: cxx_lib/different_parameters.cpp
        Line: 10*/
        // Trigger the function
        ASSERT_DEATH(first_function(a), ".*");

    }

    TEST(error, second_function_test_2)
    {
        // Construct input
        int a = -1;



        /*Error: ASSERTION FAIL: false
        File: cxx_lib/different_parameters.cpp
        Line: 19*/
        // Trigger the function
        ASSERT_DEATH(second_function(a), ".*");

    }

    TEST(error, get_sign_test_3)
    {
        // Construct input
        int a = -2147483648;



        /*Error: ASSERTION FAIL: a * sign > 0
        File: cxx_lib/different_parameters.cpp
        Line: 34*/
        // Trigger the function
        ASSERT_DEATH(get_sign(a), ".*");

    }

    TEST(error, third_function_test_1)
    {
        // Construct input
        int a = 0;



        /*Error: ASSERTION FAIL: *a > 0
        File: cxx_lib/different_parameters.cpp
        Line: 42*/
        // Trigger the function
        ASSERT_DEATH(third_function(&a), ".*");

    }

    TEST(error, third_function_test_2)
    {
        // Construct input
        int a = 1;



        /*Error: ASSERTION FAIL: *a < 0
        File: cxx_lib/different_parameters.cpp
        Line: 40*/
        // Trigger the function
        ASSERT_DEATH(third_function(&a), ".*");

    }

#pragma endregion
}
