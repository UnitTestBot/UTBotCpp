<!---
name: Supported C++ Syntax
route: /docs/cpp/advanced/cpp-syntax
parent: Documentation
menu: Advanced
description: There are numerous constructions in C++ language that should be in various ways. Here we introduce all C++ syntax that current version of UTBot supports with some examples.   
--->

# Supported C++ Syntax

Support of C++ language features is very limited. Here you can find test cases examples. All code snippets below were
taken from [this directory](https://github.com/UnitTestBot/UTBotCpp/tree/main/integration-tests/cpp-example/).

<!-- toc -->

- [Reference parameters](#reference-parameters)
- [Class as parameters](#class-as-parameters)
- [Class as return values](#class-as-return-values)
- [Class methods](#class-methods)
- [Operators](#operators)
- [Private](#private)

<!-- tocstop -->

## Reference parameters

<!---
> `additional info`

[Source code example](https://github.com/UnitTestBot/UTBotCpp/tree/main/integration-tests/c-example/lib/types/types.c#L23)
--->

```cpp
int double_abs_lvalue_reference(int& a) {
    if (a < 0) {
        a *= -2;
    }
    return a;
}

int double_abs_const_lvalue_reference(const int& a) {
    if (a < 0) {
        return -2 * a;
    }
    return a;
}
```

<details> 
  <summary>Tests code</summary>

```cpp
TEST(regression, double_abs_lvalue_reference_test_1)
{
    // Construct input
    int a = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = double_abs_lvalue_reference(a);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check function parameters
    int expected_a = 0;
    EXPECT_EQ(expected_a, a);
}

TEST(regression, double_abs_lvalue_reference_test_2)
{
    // Construct input
    int a = -1;

    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = double_abs_lvalue_reference(a);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check function parameters
    int expected_a = 2;
    EXPECT_EQ(expected_a, a);
}

TEST(regression, double_abs_const_lvalue_reference_test_1)
{
    // Construct input
    int a = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = double_abs_const_lvalue_reference(a);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, double_abs_const_lvalue_reference_test_2)
{
    // Construct input
    int a = -1;

    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = double_abs_const_lvalue_reference(a);

    // Check results
    EXPECT_EQ(expected, actual);
}
```

</details>

## Class as parameters

> For class as parameter class should be construct from initializer list of class members and has default constructor

```cpp
class Point_2d {
public:
  int x;
  int y;

  Point_2d();
  Point_2d(const int& x, const int& y);
}


void set_abs_by_ref(Point_2d& point) {
   if (point.x < 0) {
       point.x = -1 * point.x;
   }
   if (point.y < 0) {
       point.y *= -1;
   }
}
```

<details> 
  <summary>Tests code</summary>

```cpp
TEST(regression, set_abs_by_ref_test_1)
{
    // Construct input
    class Point_2d point = {-1, -1};

    // Expected output
    // No output variable for void function

    // Trigger the function
    set_abs_by_ref(point);

    // Check results
    // No check results for void function

    // Check function parameters
    class Point_2d expected_point = {1, 1};
    EXPECT_EQ(expected_point.x, point.x);
    EXPECT_EQ(expected_point.y, point.y);
}

TEST(regression, set_abs_by_ref_test_2)
{
    // Construct input
    class Point_2d point = {0, -1};

    // Expected output
    // No output variable for void function

    // Trigger the function
    set_abs_by_ref(point);

    // Check results
    // No check results for void function

    // Check function parameters
    class Point_2d expected_point = {0, 1};
    EXPECT_EQ(expected_point.x, point.x);
    EXPECT_EQ(expected_point.y, point.y);
}

TEST(regression, set_abs_by_ref_test_3)
{
    // Construct input
    class Point_2d point = {-1, 0};

    // Expected output
    // No output variable for void function

    // Trigger the function
    set_abs_by_ref(point);

    // Check results
    // No check results for void function

    // Check function parameters
    class Point_2d expected_point = {1, 0};
    EXPECT_EQ(expected_point.x, point.x);
    EXPECT_EQ(expected_point.y, point.y);
}

TEST(regression, set_abs_by_ref_test_4)
{
    // Construct input
    class Point_2d point = {0, 0};

    // Expected output
    // No output variable for void function

    // Trigger the function
    set_abs_by_ref(point);

    // Check results
    // No check results for void function

    // Check function parameters
    class Point_2d expected_point = {0, 0};
    EXPECT_EQ(expected_point.x, point.x);
    EXPECT_EQ(expected_point.y, point.y);
}
```

</details>

## Class as return values

```cpp
void set_abs_by_ref(Point_2d& point) {
   if (point.x < 0) {
       point.x = -1 * point.x;
   }
   if (point.y < 0) {
       point.y *= -1;
   }
}

Point_2d abs_point(Point_2d point) {
    set_abs_by_ref(point);
    return point;
}
```

<details>
  <summary>Tests code</summary>

```cpp
TEST(regression, abs_point_test_1)
{
    // Construct input
    class Point_2d point = {-1, 0};

    // Expected output
    class Point_2d expected = {1, 0};

    // Trigger the function
    class Point_2d actual = abs_point(point);

    // Check results
    EXPECT_EQ(expected.x, actual.x);
    EXPECT_EQ(expected.y, actual.y);
}

TEST(regression, abs_point_test_2)
{
    // Construct input
    class Point_2d point = {-1, -1};

    // Expected output
    class Point_2d expected = {1, 1};

    // Trigger the function
    class Point_2d actual = abs_point(point);

    // Check results
    EXPECT_EQ(expected.x, actual.x);
    EXPECT_EQ(expected.y, actual.y);
}

TEST(regression, abs_point_test_3)
{
    // Construct input
    class Point_2d point = {0, -1};

    // Expected output
    class Point_2d expected = {0, 1};

    // Trigger the function
    class Point_2d actual = abs_point(point);

    // Check results
    EXPECT_EQ(expected.x, actual.x);
    EXPECT_EQ(expected.y, actual.y);
}
```

</details>

## Class methods

> For genetating method tests UTBot need default constructor of class

```cpp
class Point_2d {
public:
  int x;
  int y;

  Point_2d();
  Point_2d(const int& x, const int& y);
}

int Point_2d::get_x() const {
    return x;
}
```

<details>
  <summary>Tests code</summary>

```cpp
TEST(regression, Point_2d_get_x_test_1)
{
    Point_2d Point_2d_obj;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = Point_2d_obj.get_x();

    // Check results
    EXPECT_EQ(expected, actual);
}
```

</details>

## Operators

```cpp
class Point_2d {
public:
  int x;
  int y;

  Point_2d();
  Point_2d(const int& x, const int& y);
  Point_2d operator-=(const Point_2d& rhs);
  friend Point_2d operator-(Point_2d lhs, const Point_2d& rhs);
}

Point_2d Point_2d::operator-=(const Point_2d& rhs) {
   this->x -= rhs.x;
   this->y -= rhs.y;
   return *this;
}

Point_2d operator-(Point_2d lhs, const Point_2d& rhs) {
    lhs -= rhs;
    return lhs;
}
```

<details>
  <summary>Tests code</summary>

```cpp
TEST(regression, Point_2d_operator_minus_equal_test_1)
{
    // Construct input
    Point_2d Point_2d_obj;
    class Point_2d rhs = {0, 0};

    // Expected output
    class Point_2d expected = {0, 0};

    // Trigger the function
    class Point_2d actual = Point_2d_obj.operator-=(rhs);

    // Check results
    EXPECT_EQ(expected.x, actual.x);
    EXPECT_EQ(expected.y, actual.y);
}

TEST(regression, operator_minus_test_1)
{
    // Construct input
    class Point_2d lhs = {0, 0};
    class Point_2d rhs = {0, 0};

    // Expected output
    class Point_2d expected = {0, 0};

    // Trigger the function
    class Point_2d actual = operator-(lhs, rhs);

    // Check results
    EXPECT_EQ(expected.x, actual.x);
    EXPECT_EQ(expected.y, actual.y);
}
```

</details>

## Private

> For test private UTBot use [martong/access_private.git](https://github.com/martong/access_private.git)

```cpp
class Private {
private:
    int x;
public:
    Private();
    Private(int x);
    friend Private get_abs_value(Private p);
};

Private get_abs_value(Private p) {
    if (p.x < 0) {
        return -1 * p.x;
    }
    return p;
}
```

<details>
  <summary>Tests code</summary>

```cpp
TEST(regression, get_abs_value_test_1)
{
    // Construct input
    class Private p = {-1};

    // Expected output
    class Private expected = {1};

    // Trigger the function
    class Private actual = get_abs_value(p);

    // Check results
    EXPECT_EQ(access_private::x(expected), access_private::x(actual));
}

TEST(regression, get_abs_value_test_2)
{
    // Construct input
    class Private p = {0};

    // Expected output
    class Private expected = {0};

    // Trigger the function
    class Private actual = get_abs_value(p);

    // Check results
    EXPECT_EQ(access_private::x(expected), access_private::x(actual));
}
```

</details>