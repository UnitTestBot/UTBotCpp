#include "dependent_functions_dot_c_test.h"

#include "gtest/gtest.h"
namespace UTBot {
    static const float utbot_abs_error = 1e-6;



    TEST(regression, double_max_test_1)
    {
        // Construct input
        int a = 0;
        int b = 0;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = double_max(a, b);

        // Check results
        EXPECT_EQ(expected, actual);

    }

    TEST(regression, double_max_test_2)
    {
        // Construct input
        int a = 0;
        int b = -1;

        // Expected output
        int expected = 0;

        // Trigger the function
        int actual = double_max(a, b);

        // Check results
        EXPECT_EQ(expected, actual);

    }

}
