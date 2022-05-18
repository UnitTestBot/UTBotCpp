/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#include "simple_class_dot_cpp_test.h"

#include "gtest/gtest.h"

#include "access_private.hpp"

ACCESS_PRIVATE_FIELD(Point_2d, int, x);

namespace UTBot {
  static const float utbot_abs_error = 1e-6;



  TEST(regression, Point_2d_get_dim_test_1)
  {
      Point_2d Point_2d_obj;
      unsigned int actual = Point_2d_obj.get_dim();
      EXPECT_EQ(2U, actual);
  }


  TEST(regression, Point_2d_get_x_test_1)
  {
      Point_2d Point_2d_obj;
      int actual = Point_2d_obj.get_x();
      EXPECT_EQ(0, actual);
  }


  TEST(regression, Point_2d_get_y_test_1)
  {
      Point_2d Point_2d_obj;
      int actual = Point_2d_obj.get_y();
      EXPECT_EQ(0, actual);
  }


  TEST(regression, Point_2d_operator_plus_equal_test_1)
  {
      Point_2d Point_2d_obj;
      class Point_2d rhs = {0, 0};
      class Point_2d actual = Point_2d_obj.operator+=(rhs);
      class Point_2d expected = {0, 0};
      EXPECT_EQ(access_private::x(actual), access_private::x(expected));
      EXPECT_EQ(actual.y, expected.y);
      class Point_2d expected_rhs = {0, 0};
      EXPECT_EQ(access_private::x(expected_rhs), access_private::x(rhs));
      EXPECT_EQ(expected_rhs.y, rhs.y);
  }


  TEST(regression, Point_2d_operator_minus_equal_test_1)
  {
      Point_2d Point_2d_obj;
      class Point_2d rhs = {0, 0};
      class Point_2d actual = Point_2d_obj.operator-=(rhs);
      class Point_2d expected = {0, 0};
      EXPECT_EQ(access_private::x(actual), access_private::x(expected));
      EXPECT_EQ(actual.y, expected.y);
      class Point_2d expected_rhs = {0, 0};
      EXPECT_EQ(access_private::x(expected_rhs), access_private::x(rhs));
      EXPECT_EQ(expected_rhs.y, rhs.y);
  }


  TEST(regression, Point_2d_get_dist_to_zero_test_1)
  {
      Point_2d Point_2d_obj;
      double actual = Point_2d_obj.get_dist_to_zero();
      EXPECT_NEAR(-0.000000e+00, actual, utbot_abs_error);
  }


  TEST(regression, operator_plus_test_1)
  {
      class Point_2d lhs = {0, 0};
      class Point_2d rhs = {0, 0};
      class Point_2d actual = operator+(lhs, rhs);
      class Point_2d expected = {0, 0};
      EXPECT_EQ(access_private::x(actual), access_private::x(expected));
      EXPECT_EQ(actual.y, expected.y);
      class Point_2d expected_lhs = {0, 0};
      EXPECT_EQ(access_private::x(expected_lhs), access_private::x(lhs));
      EXPECT_EQ(expected_lhs.y, lhs.y);
      class Point_2d expected_rhs = {0, 0};
      EXPECT_EQ(access_private::x(expected_rhs), access_private::x(rhs));
      EXPECT_EQ(expected_rhs.y, rhs.y);
  }


  TEST(regression, operator_minus_test_1)
  {
      class Point_2d rhs = {0, 0};
      class Point_2d actual = operator-({0, 0}, rhs);
      class Point_2d expected = {0, 0};
      EXPECT_EQ(access_private::x(actual), access_private::x(expected));
      EXPECT_EQ(actual.y, expected.y);
      class Point_2d expected_rhs = {0, 0};
      EXPECT_EQ(access_private::x(expected_rhs), access_private::x(rhs));
      EXPECT_EQ(expected_rhs.y, rhs.y);
  }


  TEST(regression, get_dist_test_1)
  {
      class Point_2d lhs = {0, 0};
      class Point_2d rhs = {0, 0};
      double actual = get_dist(lhs, rhs);
      EXPECT_NEAR(0.000000e+00, actual, utbot_abs_error);
      class Point_2d expected_lhs = {0, 0};
      EXPECT_EQ(access_private::x(expected_lhs), access_private::x(lhs));
      EXPECT_EQ(expected_lhs.y, lhs.y);
      class Point_2d expected_rhs = {0, 0};
      EXPECT_EQ(access_private::x(expected_rhs), access_private::x(rhs));
      EXPECT_EQ(expected_rhs.y, rhs.y);
  }


  TEST(regression, set_to_zero_test_1)
  {
      class Point_2d point = {-1, 2};
      set_to_zero(point);
      class Point_2d expected_point = {0, 0};
      EXPECT_EQ(access_private::x(expected_point), access_private::x(point));
      EXPECT_EQ(expected_point.y, point.y);
  }

  TEST(regression, set_to_zero_test_2)
  {
      class Point_2d point = {1, -1};
      set_to_zero(point);
      class Point_2d expected_point = {0, 0};
      EXPECT_EQ(access_private::x(expected_point), access_private::x(point));
      EXPECT_EQ(expected_point.y, point.y);
  }

  TEST(regression, set_to_zero_test_3)
  {
      class Point_2d point = {-1, -1};
      set_to_zero(point);
      class Point_2d expected_point = {0, 0};
      EXPECT_EQ(access_private::x(expected_point), access_private::x(point));
      EXPECT_EQ(expected_point.y, point.y);
  }


  TEST(regression, set_abs_by_ref_test_1)
  {
      class Point_2d point = {0, -1};
      set_abs_by_ref(point);
      class Point_2d expected_point = {0, 1};
      EXPECT_EQ(access_private::x(expected_point), access_private::x(point));
      EXPECT_EQ(expected_point.y, point.y);
  }

  TEST(regression, set_abs_by_ref_test_2)
  {
      class Point_2d point = {0, 0};
      set_abs_by_ref(point);
      class Point_2d expected_point = {0, 0};
      EXPECT_EQ(access_private::x(expected_point), access_private::x(point));
      EXPECT_EQ(expected_point.y, point.y);
  }

  TEST(regression, set_abs_by_ref_test_3)
  {
      class Point_2d point = {-1, -1};
      set_abs_by_ref(point);
      class Point_2d expected_point = {1, 1};
      EXPECT_EQ(access_private::x(expected_point), access_private::x(point));
      EXPECT_EQ(expected_point.y, point.y);
  }

}
