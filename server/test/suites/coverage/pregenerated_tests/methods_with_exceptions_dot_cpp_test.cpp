#include "methods_with_exceptions_dot_cpp_test.h"

#include "gtest/gtest.h"

#include "access_private.hpp"


namespace UTBot {
   static const float utbot_abs_error = 1e-6;


#pragma region regression
   TEST(regression, stdException_test_1)
   {
       // Construct input
       int a = 0;



       // Expected output
       int expected = 5;

       // Trigger the function
       int actual = stdException(a);

       // Check results
       EXPECT_EQ(expected, actual);
   }

   TEST(regression, badAllocException_test_1)
   {
       // Construct input
       int a = 0;



       // Expected output
       int expected = 5;

       // Trigger the function
       int actual = badAllocException(a);

       // Check results
       EXPECT_EQ(expected, actual);
   }

   TEST(regression, badCastException_test_1)
   {
       // Construct input
       int a = 0;



       // Expected output
       int expected = 5;

       // Trigger the function
       int actual = badCastException(a);

       // Check results
       EXPECT_EQ(expected, actual);
   }

   TEST(regression, optionalException_test_1)
   {
       // Construct input
       int a = 0;



       // Expected output
       int expected = 5;

       // Trigger the function
       int actual = optionalException(a);

       // Check results
       EXPECT_EQ(expected, actual);
   }

   TEST(regression, variantException_test_1)
   {
       // Construct input
       int a = 0;



       // Expected output
       int expected = 5;

       // Trigger the function
       int actual = variantException(a);

       // Check results
       EXPECT_EQ(expected, actual);
   }

   TEST(regression, customException_test_1)
   {
       // Construct input
       int a = 0;



       // Expected output
       int expected = 5;

       // Trigger the function
       int actual = customException(a);

       // Check results
       EXPECT_EQ(expected, actual);
   }

#pragma endregion
#pragma region error
   TEST(error, stdException_test_2)
   {
       // Construct input
       int a = 6;



       // Trigger the function
       EXPECT_ANY_THROW(stdException(a));

   }

   TEST(error, badAllocException_test_2)
   {
       // Construct input
       int a = 6;



       // Trigger the function
       EXPECT_ANY_THROW(badAllocException(a));

   }

   TEST(error, badCastException_test_2)
   {
       // Construct input
       int a = 6;



       // Trigger the function
       EXPECT_ANY_THROW(badCastException(a));

   }

   TEST(error, optionalException_test_2)
   {
       // Construct input
       int a = 6;



       // Trigger the function
       EXPECT_ANY_THROW(optionalException(a));

   }

   TEST(error, variantException_test_2)
   {
       // Construct input
       int a = 6;



       // Trigger the function
       EXPECT_ANY_THROW(variantException(a));

   }

   TEST(error, customException_test_2)
   {
       // Construct input
       int a = 6;



       // Trigger the function
       EXPECT_ANY_THROW(customException(a));

   }

#pragma endregion
}
