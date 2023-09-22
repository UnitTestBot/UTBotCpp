# C features
Legend:
✔️ — fully supported,
🟡 — partially supported,
❌ — not supported.
- Types
    - [Basic types](https://en.cppreference.com/w/c/language/type)
        - char ✔️ [[examples]](#char)
        - **[extended]** wchar_t ✔️ [[examples]](#wchar_t)
        - integral types ✔️  [[examples]](#integral-types)
        - size_t ✔️  [[examples]](#size_t)
        - **[extended]** __int128 / __uint128 ❌ [[examples]](#__int128--__uint128) [[discussion]](https://github.com/UnitTestBot/UTBotCpp/issues/453)
        - _Bool ✔️ [[examples]](#_bool-type)
        - floating-point types ✔️ [[examples]](#floating-point-types)
    - [Enums](https://en.cppreference.com/w/c/language/enum) ✔️ [[examples]](#enums)
    - [Arrays](https://en.cppreference.com/w/c/language/array)
        - arrays of constant known size ✔️ [[examples]](#arrays-of-constant-known-size)
        - variable-length arrays 🟡 [[examples]](#variable-length-arrays)
        - arrays of unknown size ✔️ [[examples]](#arrays-of-unknown-size)
        - multidimensional arrays 🟡 [[discussion]](https://github.com/UnitTestBot/UTBotCpp/issues/327) [[examples]](#multidimensional-arrays)
    - [Structs](https://en.cppreference.com/w/c/language/struct)
        - basic structs ✔️ [[examples]](#basic-structs)
        - structs as a return value 🟡 [[discussion]](https://github.com/UnitTestBot/UTBotCpp/issues/323)
        - anonymous structs 🟡 [[examples]](#anonymous-structs)
        - [bit fields](https://en.cppreference.com/w/c/language/bit_field) ✔️ [[examples]](#bit-fields)
    - [Unions](https://en.cppreference.com/w/c/language/union)
        - basic unions ✔️ [[examples]](#basic-unions)
        - unions as a return value 🟡 [[discussion]](https://github.com/UnitTestBot/UTBotCpp/issues/323)
        - anonymous unions 🟡 [[examples]](#anonymous-unions)
    - [Pointers](https://en.cppreference.com/w/c/language/pointer)
        - pointers as parameters ✔️ [[examples]](#pointers-as-parameters)
        - pointers as return value ✔️ [[examples]](#const)
        - null pointers ✔️ [[examples]](#null-pointers)
        - pointers to functions 🟡 [[examples]](#pointers-to-functions)
        - pointers to void ❌ [[discussion]](https://github.com/UnitTestBot/UTBotCpp/pull/221#discussion_r861876337)
        - multidimensional pointers 🟡 [[examples]](#multidimensional-pointers)
    - [Atomic types](https://en.cppreference.com/w/c/language/atomic) ❌ [[examples]](#atomic-types)
- [Functions](https://en.cppreference.com/w/c/language/functions)
    - basic functions ✔️  [[examples]](#integral-types)
    - [variadic arguments](https://en.cppreference.com/w/c/language/variadic) ❌ [[examples]](#variadic-arguments)
    - static functions ✔️ [[examples]](#static-functions)
    - [inline functions](https://en.cppreference.com/w/c/language/inline) 🟡 [[examples]](#inline-functions)
    - old-style declaration ❌ [[examples]](#old-style-function)
- [Statements and loops](https://en.cppreference.com/w/c/language/statements)
    - [if](https://en.cppreference.com/w/c/language/if) ✔️ [[examples]](#inputoutput)
    - [switch](https://en.cppreference.com/w/c/language/switch) ✔️ [[examples]](#enums)
    - [for](https://en.cppreference.com/w/c/language/for) ✔️ [[examples]](#for)
    - [while](https://en.cppreference.com/w/c/language/while) ✔️ [[examples]](#while)
    - [do-while](https://en.cppreference.com/w/c/language/do) ✔️ [[examples]](#do--while)
    - [continue](https://en.cppreference.com/w/c/language/continue) ✔️ [[examples]](#continue-break)
    - [break](https://en.cppreference.com/w/c/language/break) ✔️ [[examples]](#continue-break)
    - [goto](https://en.cppreference.com/w/c/language/goto) ✔️ [[examples]](#goto)
    - [conditional (ternary) operator](https://en.cppreference.com/w/c/language/operator_other#Conditional_operator) ❌ [[discussion]](https://github.com/UnitTestBot/UTBotCpp/issues/324)
- [Typedefs](https://en.cppreference.com/w/c/language/typedef) ✔️ [[examples]](#typedefs)
- [Storage-class specifiers](https://en.cppreference.com/w/c/language/storage_duration)
    - static variables 🟡 [[examples]](#static-variables)
    - extern variables 🟡 [[examples]](#extern-variables)
- Qualifiers ✔️
    - [const](https://en.cppreference.com/w/c/language/const) ✔️ [[examples]](#const)
    - [volatile](https://en.cppreference.com/w/c/language/volatile) ✔️ [[examples]](#volatile)
    - [restrict](https://en.cppreference.com/w/c/language/restrict) ✔️ [[examples]](#restrict)
- [Compound literals](https://en.cppreference.com/w/c/language/compound_literal) ✔️ [[examples]](#compound-literals)
- [Program execution control](https://en.cppreference.com/w/c/program)
    - [abort](https://en.cppreference.com/w/c/program/abort) ✔️ [[examples]](#abort)
    - [exit](https://en.cppreference.com/w/c/program/exit) 🟡 [[examples]](#exit)
    - [atexit](https://en.cppreference.com/w/c/program/atexit) ✔️ [[examples]](#exit)
    - [raise](https://en.cppreference.com/w/c/program/raise) 🟡 [[examples]](#raise)
- [Error handling](https://en.cppreference.com/w/c/error)
    - errno 🟡 [[examples]](#errno)
    - assert ✔️ [[examples]](#assert)
- [Dynamic memory management](https://en.cppreference.com/w/c/memory) 🟡 [[examples]](#dynamic-memory-management)
- [Input/Output](https://en.cppreference.com/w/c/io) 🟡 [[examples]](#inputoutput)
    - Files
    - StdIn/StdOut
- [Algorithms](https://en.cppreference.com/w/c/algorithm) 🟡 [[examples]](#algorithms)
- [Pseudo random number generation](https://en.cppreference.com/w/c/numeric/random) 🟡 [[examples]](#pseudo-random-number-generation)
- [Complex number arithmetic](https://en.cppreference.com/w/c/numeric/complex) ❌ [[examples]](#complex-number-arithmetic)
- [Concurrency](https://en.cppreference.com/w/c/thread) ❌ [[examples]](#atomic-types)
- [Generic selection](https://en.cppreference.com/w/c/language/generic) ✔️ [[examples]](#generic-selection)
- [Inline assembly](https://en.cppreference.com/w/c/language/asm) ✔️ [[discussion]](https://github.com/UnitTestBot/klee/pull/17)
***
# UTBot examples
> **Note**: all of the following examples can be found [here](https://github.com/UnitTestBot/UTBotCpp/tree/main/integration-tests/c-example/lib).
## Basic types
### Char
```cpp
char a_or_b(char a, char b) {
    if (a == 'a') {
        return a;
    }
    if (b == 'b') {
        return b;
    }

    if (a > b) {
        return a;
    }
    return b;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, a_or_b_test_1)
{
    char actual = a_or_b('b', 'b');
    EXPECT_EQ('b', actual);
}

TEST(regression, a_or_b_test_2)
{
    char actual = a_or_b('p', 'j');
    EXPECT_EQ('p', actual);
}

TEST(regression, a_or_b_test_3)
{
    char actual = a_or_b('i', 'n');
    EXPECT_EQ('n', actual);
}

TEST(regression, a_or_b_test_4)
{
    char actual = a_or_b('a', 'c');
    EXPECT_EQ('a', actual);
}
```
</details>

### wchar_t
```cpp
#include <wchar.h>

wchar_t wide_char(wchar_t a, wchar_t b) {
    if (b == 'z' && a > b) return a;
    if (b != 'z') return b;
    return '0';
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, wide_char_test_1)
{
    // Construct input
    wchar_t a = 0;
    wchar_t b = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = wide_char(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, wide_char_test_2)
{
    // Construct input
    wchar_t a = 0;
    wchar_t b = 122;



    // Expected output
    int expected = 48;

    // Trigger the function
    int actual = wide_char(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, wide_char_test_3)
{
    // Construct input
    wchar_t a = 123;
    wchar_t b = 122;



    // Expected output
    int expected = 123;

    // Trigger the function
    int actual = wide_char(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details> 

### Integral types

> `short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, signed char, unsigned char`

```cpp
signed long long int max_long(long long a, signed long long b) {
    if (a > b) {
        return a;
    }
    return b;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, max_long_test_1)
{
    long long actual = max_long(0LL, -1LL);
    EXPECT_EQ(0LL, actual);
}

TEST(regression, max_long_test_2)
{
    long long actual = max_long(0LL, 0LL);
    EXPECT_EQ(0LL, actual);
}
```
</details>

### size_t

```cpp
#include <stddef.h>
size_t min_size_t(size_t a, size_t b) {
    if (a < b) {
        return a;
    }
    return b;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, min_size_t_test_1)
{
    // Construct input
    size_t a = 0UL;
    size_t b = 0UL;



    // Expected output
    unsigned long expected = 0UL;

    // Trigger the function
    unsigned long actual = min_size_t(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, min_size_t_test_2)
{
    // Construct input
    size_t a = 0UL;
    size_t b = 1UL;



    // Expected output
    unsigned long expected = 0UL;

    // Trigger the function
    unsigned long actual = min_size_t(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

### __int128 / __uint128

```cpp
__uint128 foo128(__int128 sgnd) {
  if (sgnd < 0) {
    return 1;
  } else {
    __uint128 unsgnd = sgnd;
    return unsgnd;
  }
}
```

<details> 
  <summary>Generated tests</summary>

No tests generated. Couldn't find any supported methods. Please check if source directories are specified correctly. See logs for more details about unsupported functions.

LOGS: Function 'foo128' was skipped, as return type 'unsigned __int128' is not fully supported: Type is unknown

</details>

### _Bool type

> There is also a type alias bool for _Bool, defined in `<stdbool.h>`.

```cpp
int fun_that_accept_bools(_Bool a, bool b) {
    if (a && b) return 1;
    if (a) return 2;
    if (b) return 3;
    return 4;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, fun_that_accept_bools_test_1)
{
    int actual = fun_that_accept_bools(true, false);
    EXPECT_EQ(2, actual);
}

TEST(regression, fun_that_accept_bools_test_2)
{
    int actual = fun_that_accept_bools(false, true);
    EXPECT_EQ(3, actual);
}

TEST(regression, fun_that_accept_bools_test_3)
{
    int actual = fun_that_accept_bools(false, false);
    EXPECT_EQ(4, actual);
}

TEST(regression, fun_that_accept_bools_test_4)
{
    int actual = fun_that_accept_bools(true, true);
    EXPECT_EQ(1, actual);
}
```
</details>

### Floating-point types

> `double, float, long double`

```cpp
float long_double_arith(long double x) {
    x *= 2;
    x -= 3.21;
    x *= fabsl(x);
    if (x == 1.0) {
        return 1.0;
    } else {
        return 3.5;
    }
}
```
<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, long_double_arith_test_1)
{
    float actual = long_double_arith(2.105000e+00);
    EXPECT_NEAR(1.000000e+00, actual, utbot_abs_error);
}

TEST(regression, long_double_arith_test_2)
{
    float actual = long_double_arith(0.000000e+00);
    EXPECT_NEAR(3.500000e+00, actual, utbot_abs_error);
}
```
</details> 

```cpp
int plain_isnan(float x) {
    if (x != x) {
        return 1;
    } else {
        return 0;
    }
}
```
<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, plain_isnan_test_1)
{
    int actual = plain_isnan(NAN);
    EXPECT_EQ(1, actual);
}

TEST(regression, plain_isnan_test_2)
{
    int actual = plain_isnan(0.000000e+00);
    EXPECT_EQ(0, actual);
}
```
</details>

## Pointers

> If a pointer is used as a return value, UTBot is not yet capable of determining if it is used as an array, so only value under the pointer itself will be checked in generated tests.

### pointers as parameters

```cpp
int c_strcmp(const char* a, const char *b) {
    for (int i = 0; ; i++) {
        if (a[i] != b[i]) {
            return 0;
        } else {
            if (a[i] == '\0' || b[i] == '\0') {
                return a[i] == '\0' && b[i] == '\0';
            }
        }
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, c_strcmp_test_1)
{
    // Construct input
    char a_buffer[] = "ccacacccc";
    const char * a = a_buffer;
    char b_buffer[] = "ccacacccc";
    const char * b = b_buffer;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = c_strcmp(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, c_strcmp_test_2)
{
    // Construct input
    char a_buffer[] = "iccccaccc";
    const char * a = a_buffer;
    char b_buffer[] = "acbcbcccc";
    const char * b = b_buffer;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = c_strcmp(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, c_strcmp_test_3)
{
    // Construct input
    char a_buffer[] = "c";
    const char * a = a_buffer;
    char b_buffer[] = "c";
    const char * b = b_buffer;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = c_strcmp(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, c_strcmp_test_4)
{
    // Construct input
    char a_buffer[] = "cbcccaccc";
    const char * a = a_buffer;
    char b_buffer[] = "crbcbcccc";
    const char * b = b_buffer;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = c_strcmp(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, c_strcmp_test_5)
{
    // Construct input
    char a_buffer[] = "";
    const char * a = a_buffer;
    char b_buffer[] = "";
    const char * b = b_buffer;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = c_strcmp(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
int longptr_cmp(long *a, long *b) {
    return (*a == *b);
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, longptr_cmp_test_1)
{
    // Construct input
    long a = 0L;
    long b = 0L;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = longptr_cmp(&a, &b);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check function parameters
    long expected_a = 0L;
    EXPECT_EQ(expected_a, a);
    long expected_b = 0L;
    EXPECT_EQ(expected_b, b);
}
```
</details>

### null pointers

```cpp
int* return_nullptr(int x) {
    static int ret = 5;
    static int nine = 9;
    if (x == 11) {
        return &nine;
    } else if (x == 0) {
        return NULL;
    } else {
        return &ret;
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, return_nullptr_test_1)
{
    // Construct input
    int x = 2;



    // Expected output
    int expected = 5;

    // Trigger the function
    int actual = *return_nullptr(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, return_nullptr_test_2)
{
    // Construct input
    int x = 0;



    // Expected output
    // No output variable check for function returning null

    // Trigger the function
    int* actual = return_nullptr(x);

    // Check results
    EXPECT_TRUE(actual == NULL);
}

TEST(regression, return_nullptr_test_3)
{
    // Construct input
    int x = 11;



    // Expected output
    int expected = 9;

    // Trigger the function
    int actual = *return_nullptr(x);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

### pointers to functions

> * If return type is a pointer to function, UTBot doesn't checking expected value - comparing pointers doesn't make any sense.
> * We support arrays of pointers to functions also, but 1-dimensional only.
> * If a function takes pointer to another function as parameter, UTBot generates stub for this parameter.

> Known issues: [[#331](https://github.com/UnitTestBot/UTBotCpp/issues/331)]

```cpp
int receiver(int f(int, int), char c) {
    if (c == 'a') {
        return f(2, 3);
    } else if (c == 'b') {
        return f(4, 5) + 8;
    } else {
        return -1;
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, receiver_test_1)
{
    // Construct input
    receiver_f_arg f = *_receiver_f_stub;
    char c = 'c';



    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = receiver(f, c);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, receiver_test_2)
{
    // Construct input
    receiver_f_arg f = *_receiver_f_stub;
    char c = 'b';



    // Expected output
    int expected = 8;

    // Trigger the function
    int actual = receiver(f, c);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, receiver_test_3)
{
    // Construct input
    receiver_f_arg f = *_receiver_f_stub;
    char c = 'a';



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = receiver(f, c);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

### multidimensional pointers
> Only 1d and 2d pointers are supported.

> Known issues: [[327](https://github.com/UnitTestBot/UTBotCpp/issues/327)]

```cpp
int some_method(int **pointer2d) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            if (pointer2d[i][j] > 0) {
                return i * 2 + j;
            }
        }
    }
    return -1;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, some_method_test_1)
{
    // Construct input
    int _pointer2d[2][2] = {{0, 0}, {0, 0}};
    int ** pointer2d = (int **) calloc(3, sizeof(int *));
    for (int it_14_0 = 0; it_14_0 < 2; it_14_0 ++) {
        pointer2d[it_14_0] = _pointer2d[it_14_0];
    }
    pointer2d[2] = NULL;



    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = some_method(pointer2d);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check function parameters
    int expected_pointer2d[2][2] = {{0, 0}, {0, 0}};
    for (int it_15_0 = 0; it_15_0 < 2; it_15_0 ++) {
        for (int it_15_1 = 0; it_15_1 < 2; it_15_1 ++) {
            EXPECT_EQ(expected_pointer2d[it_15_0][it_15_1], _pointer2d[it_15_0][it_15_1]);
        }
    }
}

TEST(regression, some_method_test_2)
{
    // Construct input
    int _pointer2d[2][2] = {{0, 0}, {1, 0}};
    int ** pointer2d = (int **) calloc(3, sizeof(int *));
    for (int it_16_0 = 0; it_16_0 < 2; it_16_0 ++) {
        pointer2d[it_16_0] = _pointer2d[it_16_0];
    }
    pointer2d[2] = NULL;



    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = some_method(pointer2d);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check function parameters
    int expected_pointer2d[2][2] = {{0, 0}, {1, 0}};
    for (int it_17_0 = 0; it_17_0 < 2; it_17_0 ++) {
        for (int it_17_1 = 0; it_17_1 < 2; it_17_1 ++) {
            EXPECT_EQ(expected_pointer2d[it_17_0][it_17_1], _pointer2d[it_17_0][it_17_1]);
        }
    }
}

TEST(regression, some_method_test_3)
{
    // Construct input
    int _pointer2d[2][2] = {{0, 1}, {0, 0}};
    int ** pointer2d = (int **) calloc(3, sizeof(int *));
    for (int it_18_0 = 0; it_18_0 < 2; it_18_0 ++) {
        pointer2d[it_18_0] = _pointer2d[it_18_0];
    }
    pointer2d[2] = NULL;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = some_method(pointer2d);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check function parameters
    int expected_pointer2d[2][2] = {{0, 1}, {0, 0}};
    for (int it_19_0 = 0; it_19_0 < 2; it_19_0 ++) {
        for (int it_19_1 = 0; it_19_1 < 2; it_19_1 ++) {
            EXPECT_EQ(expected_pointer2d[it_19_0][it_19_1], _pointer2d[it_19_0][it_19_1]);
        }
    }
}

TEST(regression, some_method_test_4)
{
    // Construct input
    int _pointer2d[2][2] = {{1, 0}, {0, 0}};
    int ** pointer2d = (int **) calloc(3, sizeof(int *));
    for (int it_20_0 = 0; it_20_0 < 2; it_20_0 ++) {
        pointer2d[it_20_0] = _pointer2d[it_20_0];
    }
    pointer2d[2] = NULL;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = some_method(pointer2d);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check function parameters
    int expected_pointer2d[2][2] = {{1, 0}, {0, 0}};
    for (int it_21_0 = 0; it_21_0 < 2; it_21_0 ++) {
        for (int it_21_1 = 0; it_21_1 < 2; it_21_1 ++) {
            EXPECT_EQ(expected_pointer2d[it_21_0][it_21_1], _pointer2d[it_21_0][it_21_1]);
        }
    }
}
```
</details>

### Atomic types

```cpp
// need to be linked with "-latomic"

#include <stdatomic.h>
#include <stdio.h>
 
_Atomic struct A { int a[100]; } a;
_Atomic struct B { int x, y; } b;

int atomic_test(void) {
  int res = 0;
  if (atomic_is_lock_free(&a)) {
    res |= 1;
  }
  if (atomic_is_lock_free(&b)) {
    res |= 2;
  }
  printf("_Atomic struct A is lock free? %s\n", 
          atomic_is_lock_free(&a) ? "true" : "false");
  printf("_Atomic struct B is lock free? %s\n", 
          atomic_is_lock_free(&b) ? "true" : "false");
  return res;
}
```

<details> 
  <summary>Generated tests</summary>

Local at [d1ead3c](https://github.com/UnitTestBot/UTBotCpp/commit/d1ead3c5a00b05380b59fc056f63ed024cdf9926): Unexpected error in RPC handling
Online at https://www.utbot.org/utbot/: [LOGS](https://pastebin.com/UMAYbcFm)

</details>


## Functions
### variadic arguments

> Tests are generated, but they don't use variadic at all.

```cpp
int adder_plus_plus(size_t nargs, int n1, int n2, ...) {
    va_list args;
    va_start(args, nargs);
    int sum = n1 + n2;
    for (size_t i = 0; i < nargs; ++i) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
#pragma region regression
TEST(regression, adder_plus_plus_test_1)
{
    // Construct input
    size_t nargs = 0UL;
    int n1 = 0;
    int n2 = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = adder_plus_plus(nargs, n1, n2);

    // Check results
    EXPECT_EQ(expected, actual);
}

#pragma endregion

#pragma region error
TEST(error, adder_plus_plus_test_2)
{
    // Construct input
    size_t nargs = 1UL;
    int n1 = 0;
    int n2 = 0;



    // Trigger the function
    adder_plus_plus(nargs, n1, n2);

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}
#pragma endregion
```
</details>

### static functions

```cpp
static int x = 0;

static int static_simple(int dx)
{
    if (x > 0) {
        return x + dx;
    } else if (x < 0) {
        return -x + dx;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, static_simple_test_1)
{
    // Initialize global variables
    x = 0;

    // Construct input
    int dx = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_simple(dx);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check global variables
    int expected_x = 0;
    EXPECT_EQ(expected_x, x);
}

TEST(regression, static_simple_test_2)
{
    // Initialize global variables
    x = -10;

    // Construct input
    int dx = -10;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_simple(dx);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check global variables
    int expected_x = -10;
    EXPECT_EQ(expected_x, x);
}

TEST(regression, static_simple_test_3)
{
    // Initialize global variables
    x = 1;

    // Construct input
    int dx = -1;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_simple(dx);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check global variables
    int expected_x = 1;
    EXPECT_EQ(expected_x, x);
}
```
</details>

### inline functions

> `inline` functions without `static` or `extern` specifier is not supported by now

```cpp
static inline int static_inline_sum(int a, int b) {
    return a + b;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, static_inline_sum_test_1)
{
    // Construct input
    int a = 0;
    int b = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_inline_sum(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

### Old-style function
> doesn't work now, see [#495](../../issues/495)

```c
int sum(a, b)
int a;
int b;
{
    return a + b;
}
```


## Enums

```cpp
enum Sign {
    NEGATIVE,
    ZERO,
    POSITIVE
};

int getSignValue(enum Sign s) {
    switch (s) {
        case NEGATIVE:
            return 0;
        case ZERO:
            return 1;
        case POSITIVE:
            return 2;
        default:
            return -1;
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, getSignValue_test_1)
{
    int actual = getSignValue(NEGATIVE);
    EXPECT_EQ(0, actual);
}

TEST(regression, getSignValue_test_2)
{
    int actual = getSignValue(ZERO);
    EXPECT_EQ(1, actual);
}

TEST(regression, getSignValue_test_3)
{
    int actual = getSignValue(POSITIVE);
    EXPECT_EQ(2, actual);
}
```
</details> 

## Arrays
> Arrays of any dimensions are supported, but as for pointers, only 1-d and 2-d are supported.
### arrays of constant known size

```cpp
int sum_sign(int a[2][2]) {
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            sum += a[i][j];
        }
    }
    if (sum == 0) {
        return 0;
    } else if (sum > 0) {
        return 1;
    } else {
        return -1;
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, sum_sign_test_1)
{
    int a[2][2] = {{4, 5}, {-3, -7}};
    int actual = sum_sign(a);
    EXPECT_EQ(-1, actual);
    int expected_a[2][2] = {{4, 5}, {-3, -7}};
    for (int it_40_0 = 0; it_40_0 < 2; it_40_0 ++) {
        for (int it_40_1 = 0; it_40_1 < 2; it_40_1 ++) {
            EXPECT_EQ(expected_a[it_40_0][it_40_1], a[it_40_0][it_40_1]);
        }
    }
}

TEST(regression, sum_sign_test_2)
{
    int a[2][2] = {{0, 7}, {8, 1}};
    int actual = sum_sign(a);
    EXPECT_EQ(1, actual);
    int expected_a[2][2] = {{0, 7}, {8, 1}};
    for (int it_41_0 = 0; it_41_0 < 2; it_41_0 ++) {
        for (int it_41_1 = 0; it_41_1 < 2; it_41_1 ++) {
            EXPECT_EQ(expected_a[it_41_0][it_41_1], a[it_41_0][it_41_1]);
        }
    }
}

TEST(regression, sum_sign_test_3)
{
    int a[2][2] = {{0, 0}, {0, 0}};
    int actual = sum_sign(a);
    EXPECT_EQ(0, actual);
    int expected_a[2][2] = {{0, 0}, {0, 0}};
    for (int it_42_0 = 0; it_42_0 < 2; it_42_0 ++) {
        for (int it_42_1 = 0; it_42_1 < 2; it_42_1 ++) {
            EXPECT_EQ(expected_a[it_42_0][it_42_1], a[it_42_0][it_42_1]);
        }
    }
}
```
</details>

### variable-length arrays
>Tests for the first function are weird a bit, which can be connected with [issue](https://github.com/UnitTestBot/UTBotCpp/issues/327)
```cpp
size_t variable_length(size_t len) {
    if (len > 100 || len == 0) {
        return 0;
    }
    size_t a[len];
    for (size_t i = 0; i < len; ++i) {
        a[i] = i + 1;
    }
    if (a[len / 2] == 3) {
        return 1;
    }
    return 2;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
#pragma region regression
TEST(regression, variable_length_test_1)
{
    unsigned long actual = variable_length(4UL);
    EXPECT_EQ(1UL, actual);
}

TEST(regression, variable_length_test_2)
{
    unsigned long actual = variable_length(0UL);
    EXPECT_EQ(0UL, actual);
}

TEST(regression, variable_length_test_3)
{
    unsigned long actual = variable_length(104UL);
    EXPECT_EQ(0UL, actual);
}

#pragma endregion
#pragma region error
TEST(error, variable_length_test_4)
{
    variable_length(2UL);
}

#pragma endregion
```
</details>

```cpp
size_t variable_length_2(size_t len, unsigned int a[len]) {
    if (len > 10 || len == 0) {
        return 0;
    }
    for (size_t i = 0; i < len; ++i) {
        a[i] = i + 1;
    }
    if (a[len - 1] == 3) {
        return 1;
    }
    return 2;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, variable_length_2_test_1)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned long actual = variable_length_2(1UL, a);
    EXPECT_EQ(2UL, actual);
    unsigned int expected_a[10] = {1U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_70_0 = 0; it_70_0 < 10; it_70_0 ++) {
        EXPECT_EQ(expected_a[it_70_0], a[it_70_0]);
    }
}

TEST(regression, variable_length_2_test_2)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned long actual = variable_length_2(18374686479671623680UL, a);
    EXPECT_EQ(0UL, actual);
    unsigned int expected_a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_71_0 = 0; it_71_0 < 10; it_71_0 ++) {
        EXPECT_EQ(expected_a[it_71_0], a[it_71_0]);
    }
}

TEST(regression, variable_length_2_test_3)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned long actual = variable_length_2(3UL, a);
    EXPECT_EQ(1UL, actual);
    unsigned int expected_a[10] = {1U, 2U, 3U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_72_0 = 0; it_72_0 < 10; it_72_0 ++) {
        EXPECT_EQ(expected_a[it_72_0], a[it_72_0]);
    }
}

TEST(regression, variable_length_2_test_4)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned long actual = variable_length_2(10UL, a);
    EXPECT_EQ(2UL, actual);
    unsigned int expected_a[10] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U};
    for (int it_73_0 = 0; it_73_0 < 10; it_73_0 ++) {
        EXPECT_EQ(expected_a[it_73_0], a[it_73_0]);
    }
}

TEST(regression, variable_length_2_test_5)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned long actual = variable_length_2(0UL, a);
    EXPECT_EQ(0UL, actual);
    unsigned int expected_a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_74_0 = 0; it_74_0 < 10; it_74_0 ++) {
        EXPECT_EQ(expected_a[it_74_0], a[it_74_0]);
    }
}
```
</details>

### arrays of unknown size

```cpp
unsigned int unknown_size(size_t len, unsigned int a[]) {
    if (len > 5 || len == 0) {
        return 0;
    }
    for (size_t i = 0; i < len; ++i) {
        a[i] = i + 1;
    }
    if (a[len - 1] == 3) {
        return 1;
    }
    return 2;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, unknown_size_test_1)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned int actual = unknown_size(1UL, a);
    EXPECT_EQ(2U, actual);
    unsigned int expected_a[10] = {1U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_23_0 = 0; it_23_0 < 10; it_23_0 ++) {
        EXPECT_EQ(expected_a[it_23_0], a[it_23_0]);
    }
}

TEST(regression, unknown_size_test_2)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned int actual = unknown_size(8UL, a);
    EXPECT_EQ(0U, actual);
    unsigned int expected_a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_24_0 = 0; it_24_0 < 10; it_24_0 ++) {
        EXPECT_EQ(expected_a[it_24_0], a[it_24_0]);
    }
}

TEST(regression, unknown_size_test_3)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned int actual = unknown_size(3UL, a);
    EXPECT_EQ(1U, actual);
    unsigned int expected_a[10] = {1U, 2U, 3U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_25_0 = 0; it_25_0 < 10; it_25_0 ++) {
        EXPECT_EQ(expected_a[it_25_0], a[it_25_0]);
    }
}

TEST(regression, unknown_size_test_4)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned int actual = unknown_size(5UL, a);
    EXPECT_EQ(2U, actual);
    unsigned int expected_a[10] = {1U, 2U, 3U, 4U, 5U, 0U, 0U, 0U, 0U, 0U};
    for (int it_26_0 = 0; it_26_0 < 10; it_26_0 ++) {
        EXPECT_EQ(expected_a[it_26_0], a[it_26_0]);
    }
}

TEST(regression, unknown_size_test_5)
{
    unsigned int a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    unsigned int actual = unknown_size(0UL, a);
    EXPECT_EQ(0U, actual);
    unsigned int expected_a[10] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U};
    for (int it_27_0 = 0; it_27_0 < 10; it_27_0 ++) {
        EXPECT_EQ(expected_a[it_27_0], a[it_27_0]);
    }
}
```
</details>

### multidimensional arrays

```cpp
int value(int a[2][3]) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            if (a[i][j] > 0) {
                return 3 * i + j;
            }
        }
    }
    return -1;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, value_test_1)
{
    int a[2][3] = {{0, 0, 0}, {0, 0, 0}};
    int actual = value(a);
    EXPECT_EQ(-1, actual);
    int expected_a[2][3] = {{0, 0, 0}, {0, 0, 0}};
    for (int it_63_0 = 0; it_63_0 < 2; it_63_0 ++) {
        for (int it_63_1 = 0; it_63_1 < 3; it_63_1 ++) {
            EXPECT_EQ(expected_a[it_63_0][it_63_1], a[it_63_0][it_63_1]);
        }
    }
}

TEST(regression, value_test_2)
{
    int a[2][3] = {{0, 0, 0}, {1, 0, 0}};
    int actual = value(a);
    EXPECT_EQ(3, actual);
    int expected_a[2][3] = {{0, 0, 0}, {1, 0, 0}};
    for (int it_64_0 = 0; it_64_0 < 2; it_64_0 ++) {
        for (int it_64_1 = 0; it_64_1 < 3; it_64_1 ++) {
            EXPECT_EQ(expected_a[it_64_0][it_64_1], a[it_64_0][it_64_1]);
        }
    }
}

TEST(regression, value_test_3)
{
    int a[2][3] = {{0, 1, 0}, {0, 0, 0}};
    int actual = value(a);
    EXPECT_EQ(1, actual);
    int expected_a[2][3] = {{0, 1, 0}, {0, 0, 0}};
    for (int it_65_0 = 0; it_65_0 < 2; it_65_0 ++) {
        for (int it_65_1 = 0; it_65_1 < 3; it_65_1 ++) {
            EXPECT_EQ(expected_a[it_65_0][it_65_1], a[it_65_0][it_65_1]);
        }
    }
}

TEST(regression, value_test_4)
{
    int a[2][3] = {{1, 0, 0}, {0, 0, 0}};
    int actual = value(a);
    EXPECT_EQ(0, actual);
    int expected_a[2][3] = {{1, 0, 0}, {0, 0, 0}};
    for (int it_66_0 = 0; it_66_0 < 2; it_66_0 ++) {
        for (int it_66_1 = 0; it_66_1 < 3; it_66_1 ++) {
            EXPECT_EQ(expected_a[it_66_0][it_66_1], a[it_66_0][it_66_1]);
        }
    }
}
```
</details>

```cpp
#define W 3
#define H 2
static int matrix_a[W][H];
static int matrix_b[W][H];
static int matrix_c[W][H];

int sum_matrix() {
    int sum = 0;
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < H; j++) {
            matrix_c[i][j] = matrix_a[i][j] + matrix_b[i][j];
            sum += matrix_c[i][j];
        }
    }
    if (sum < 0) {
        return sum;
    }
    if (sum == 0) {
        return 0;
    }
    return sum;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, sum_matrix_test_1)
{
    int matrix_c_buffer[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    memcpy((void *) matrix_c, matrix_c_buffer, sizeof(matrix_c_buffer));
    int matrix_a_buffer[3][2] = {{2, 8}, {-1, -1}, {-2, -1}};
    memcpy((void *) matrix_a, matrix_a_buffer, sizeof(matrix_a_buffer));
    int matrix_b_buffer[3][2] = {{-1, -2}, {-1, -1}, {-2, 5}};
    memcpy((void *) matrix_b, matrix_b_buffer, sizeof(matrix_b_buffer));
    int actual = sum_matrix();
    EXPECT_EQ(3, actual);
    int expected_matrix_c[3][2] = {{1, 6}, {-2, -2}, {-4, 4}};
    for (int it_91_0 = 0; it_91_0 < 3; it_91_0 ++) {
        for (int it_91_1 = 0; it_91_1 < 2; it_91_1 ++) {
            EXPECT_EQ(expected_matrix_c[it_91_0][it_91_1], matrix_c[it_91_0][it_91_1]);
        }
    }
    int expected_matrix_a[3][2] = {{2, 8}, {-1, -1}, {-2, -1}};
    for (int it_92_0 = 0; it_92_0 < 3; it_92_0 ++) {
        for (int it_92_1 = 0; it_92_1 < 2; it_92_1 ++) {
            EXPECT_EQ(expected_matrix_a[it_92_0][it_92_1], matrix_a[it_92_0][it_92_1]);
        }
    }
    int expected_matrix_b[3][2] = {{-1, -2}, {-1, -1}, {-2, 5}};
    for (int it_93_0 = 0; it_93_0 < 3; it_93_0 ++) {
        for (int it_93_1 = 0; it_93_1 < 2; it_93_1 ++) {
            EXPECT_EQ(expected_matrix_b[it_93_0][it_93_1], matrix_b[it_93_0][it_93_1]);
        }
    }
}

TEST(regression, sum_matrix_test_2)
{
    int matrix_c_buffer[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    memcpy((void *) matrix_c, matrix_c_buffer, sizeof(matrix_c_buffer));
    int matrix_a_buffer[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    memcpy((void *) matrix_a, matrix_a_buffer, sizeof(matrix_a_buffer));
    int matrix_b_buffer[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    memcpy((void *) matrix_b, matrix_b_buffer, sizeof(matrix_b_buffer));
    int actual = sum_matrix();
    EXPECT_EQ(0, actual);
    int expected_matrix_c[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    for (int it_94_0 = 0; it_94_0 < 3; it_94_0 ++) {
        for (int it_94_1 = 0; it_94_1 < 2; it_94_1 ++) {
            EXPECT_EQ(expected_matrix_c[it_94_0][it_94_1], matrix_c[it_94_0][it_94_1]);
        }
    }
    int expected_matrix_a[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    for (int it_95_0 = 0; it_95_0 < 3; it_95_0 ++) {
        for (int it_95_1 = 0; it_95_1 < 2; it_95_1 ++) {
            EXPECT_EQ(expected_matrix_a[it_95_0][it_95_1], matrix_a[it_95_0][it_95_1]);
        }
    }
    int expected_matrix_b[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    for (int it_96_0 = 0; it_96_0 < 3; it_96_0 ++) {
        for (int it_96_1 = 0; it_96_1 < 2; it_96_1 ++) {
            EXPECT_EQ(expected_matrix_b[it_96_0][it_96_1], matrix_b[it_96_0][it_96_1]);
        }
    }
}

TEST(regression, sum_matrix_test_3)
{
    int matrix_c_buffer[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    memcpy((void *) matrix_c, matrix_c_buffer, sizeof(matrix_c_buffer));
    int matrix_a_buffer[3][2] = {{-1, 0}, {0, 0}, {0, 0}};
    memcpy((void *) matrix_a, matrix_a_buffer, sizeof(matrix_a_buffer));
    int matrix_b_buffer[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    memcpy((void *) matrix_b, matrix_b_buffer, sizeof(matrix_b_buffer));
    int actual = sum_matrix();
    EXPECT_EQ(-1, actual);
    int expected_matrix_c[3][2] = {{-1, 0}, {0, 0}, {0, 0}};
    for (int it_97_0 = 0; it_97_0 < 3; it_97_0 ++) {
        for (int it_97_1 = 0; it_97_1 < 2; it_97_1 ++) {
            EXPECT_EQ(expected_matrix_c[it_97_0][it_97_1], matrix_c[it_97_0][it_97_1]);
        }
    }
    int expected_matrix_a[3][2] = {{-1, 0}, {0, 0}, {0, 0}};
    for (int it_98_0 = 0; it_98_0 < 3; it_98_0 ++) {
        for (int it_98_1 = 0; it_98_1 < 2; it_98_1 ++) {
            EXPECT_EQ(expected_matrix_a[it_98_0][it_98_1], matrix_a[it_98_0][it_98_1]);
        }
    }
    int expected_matrix_b[3][2] = {{0, 0}, {0, 0}, {0, 0}};
    for (int it_99_0 = 0; it_99_0 < 3; it_99_0 ++) {
        for (int it_99_1 = 0; it_99_1 < 2; it_99_1 ++) {
            EXPECT_EQ(expected_matrix_b[it_99_0][it_99_1], matrix_b[it_99_0][it_99_1]);
        }
    }
}
```
</details>

## Structs
### basic structs

```cpp
int get_sign_struct(struct MyStruct st) {
    if (st.a == 0) {
        return 0;
    }
    if (st.a < 0) {
        return -1;
    } else {
        return 1;
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, get_sign_struct_test_1)
{
    int actual = get_sign_struct({
        .x = 0,
        .a = 1});
    EXPECT_EQ(1, actual);
}

TEST(regression, get_sign_struct_test_2)
{
    int actual = get_sign_struct({
        .x = 0,
        .a = -10});
    EXPECT_EQ(-1, actual);
}

TEST(regression, get_sign_struct_test_3)
{
    int actual = get_sign_struct({
        .x = 0,
        .a = 0});
    EXPECT_EQ(0, actual);
}
```
</details>

### anonymous structs

>Supported, but tests are generated using `from_bytes`, which is not quite convenient.

```cpp
struct WithAnonymous {
    struct { int x, y };
    int m;
};

int count_equal_members(struct WithAnonymous st) {
    if (st.x == st.y && st.x == st.m) {
        return 3;
    } else if (st.x == st.y || st.x == st.m || st.y == st.m) {
        return 2;
    }
    return 1;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, count_equal_members_test_1)
{
    int actual = count_equal_members(from_bytes<WithAnonymous>({4, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0}));
    EXPECT_EQ(1, actual);
}

TEST(regression, count_equal_members_test_2)
{
    int actual = count_equal_members(from_bytes<WithAnonymous>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
    EXPECT_EQ(3, actual);
}

TEST(regression, count_equal_members_test_3)
{
    int actual = count_equal_members(from_bytes<WithAnonymous>({0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0}));
    EXPECT_EQ(2, actual);
}

TEST(regression, count_equal_members_test_4)
{
    int actual = count_equal_members(from_bytes<WithAnonymous>({0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0}));
    EXPECT_EQ(2, actual);
}

TEST(regression, count_equal_members_test_5)
{
    int actual = count_equal_members(from_bytes<WithAnonymous>({0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}));
    EXPECT_EQ(2, actual);
}
```
</details>

### bit fields

```cpp
typedef struct {
    signed a : 24;
    signed b : 1;
    signed int c : 2;
    signed int d : 5;
} SimpleSignedStr;

int check_simple_signed_str(SimpleSignedStr s) {
    if (s.a == 1024 && s.b == -1 && s.d == -16) {
        return 1;
    } else if (s.b == 0) {
        return -1;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, check_simple_signed_str_test_1)
{
    // Construct input
    SimpleSignedStr s = {
        .a = 0,
        .b = 0,
        .c = 0,
        .d = 0
    };

    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = check_simple_signed_str(s);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_simple_signed_str_test_2)
{
    // Construct input
    SimpleSignedStr s = {
        .a = 1024,
        .b = -1,
        .c = 0,
        .d = 0
    };

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_simple_signed_str(s);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_simple_signed_str_test_3)
{
    // Construct input
    SimpleSignedStr s = {
        .a = 0,
        .b = -1,
        .c = 0,
        .d = 0
    };

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = check_simple_signed_str(s);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_simple_signed_str_test_4)
{
    // Construct input
    SimpleSignedStr s = {
        .a = 1024,
        .b = -1,
        .c = 0,
        .d = -16
    };

    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = check_simple_signed_str(s);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, check_simple_signed_str_test_5)
{
    // Construct input
    SimpleSignedStr s = {
        .a = 1024,
        .b = 0,
        .c = 0,
        .d = 0
    };

    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = check_simple_signed_str(s);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
typedef struct {
    unsigned b1 : 7;
    unsigned : 0;
    unsigned b2 : 6;
    unsigned : 3;
    unsigned b3 : 15;
} StrWithUnnamedZeroBitfield;

int is_nice(StrWithUnnamedZeroBitfield s) {
    if (s.b1 == 69 && s.b2 == 42 && s.b3 == 1488) {
        return 13;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, is_nice_test_1)
{
    // Construct input
    StrWithUnnamedZeroBitfield s = {
        .b1 = 0U,
        .b2 = 0U,
        .b3 = 0U
    };

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = is_nice(s);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, is_nice_test_2)
{
    // Construct input
    StrWithUnnamedZeroBitfield s = {
        .b1 = 69U,
        .b2 = 0U,
        .b3 = 0U
    };

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = is_nice(s);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, is_nice_test_3)
{
    // Construct input
    StrWithUnnamedZeroBitfield s = {
        .b1 = 69U,
        .b2 = 42U,
        .b3 = 0U
    };

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = is_nice(s);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, is_nice_test_4)
{
    // Construct input
    StrWithUnnamedZeroBitfield s = {
        .b1 = 69U,
        .b2 = 42U,
        .b3 = 1488U
    };

    // Expected output
    int expected = 13;

    // Trigger the function
    int actual = is_nice(s);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

## Unions
### basic unions

```cpp
union IntBytesUnion {
    char bytes[4];
    int number;
};

int get_sign_union(union IntBytesUnion st) {
    if (st.number == 0) {
        return 0;
    }
    if (st.number < 0) {
        return -1;
    } else {
        return 1;
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, get_sign_union_test_1)
{
    int actual = get_sign_union({
        .bytes = {'\0', '\0', '\0', '\0'}
        // .number = 0
    });
    EXPECT_EQ(0, actual);
}

TEST(regression, get_sign_union_test_2)
{
    int actual = get_sign_union({
        .bytes = {'p', 'b', 'b', '\x80'}
        // .number = -2141035920
    });
    EXPECT_EQ(-1, actual);
}

TEST(regression, get_sign_union_test_3)
{
    int actual = get_sign_union({
        .bytes = {'p', 'p', 'p', 'b'}
        // .number = 1651535984
    });
    EXPECT_EQ(1, actual);
}

```
</details>

### anonymous unions

> Supported, but tests are generated using `from_bytes`, which is not quite convenient. #447

```cpp
struct WithAnonymous {
   union {
      int i, j;
   };
   int m;
};

int count_equal_members(struct WithAnonymous st) {
    if (st.i == st.m) {
        return 2;
    }
    return 1;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, count_equal_members_test_1)
{
    int actual = count_equal_members(from_bytes<WithAnonymous>({0, 0, 0, 0, 4, 0, 0, 0}));
    EXPECT_EQ(1, actual);
}

TEST(regression, count_equal_members_test_2)
{
    int actual = count_equal_members(from_bytes<WithAnonymous>({0, 0, 0, 0, 0, 0, 0, 0}));
    EXPECT_EQ(2, actual);
}
```
</details>

## Statements and loops
### for

```cpp
int for_loop(int n) {
  int sum = 0;
  for (int i = 1; i <= n; ++i) {
    sum += i;
  }
  if (sum == 21) {
    return n;
  }
  if (sum < 10) {
    return n * 100;
  }
  return 2;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, for_loop_test_1)
{
    int actual = for_loop(0);
    EXPECT_EQ(0, actual);
}

TEST(regression, for_loop_test_2)
{
    int actual = for_loop(6);
    EXPECT_EQ(6, actual);
}

TEST(regression, for_loop_test_3)
{
    int actual = for_loop(4);
    EXPECT_EQ(2, actual);
}
```
</details> 

### while

```cpp
int while_loop(int n) {
    int i = 0;
    while (i < n) {
        i = i + 1;
        if (n % i == 37)
            return 1;
        else if (i == 50)
            return 2;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, while_loop_test_1)
{
    int actual = while_loop(0);
    EXPECT_EQ(0, actual);
}

TEST(regression, while_loop_test_2)
{
    int actual = while_loop(536870912);
    EXPECT_EQ(2, actual);
}

TEST(regression, while_loop_test_3)
{
    int actual = while_loop(2111107707);
    EXPECT_EQ(1, actual);
}
```
</details> 

### do .. while

```cpp
int do_while_loop(int n) {
    int i = 0;
    do {
        i = i + 1;
        if (n % i == 37)
            return 1;
        else if (i == 50)
            return 2;
    } while (i < n);
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, do_while_loop_test_1)
{
    int actual = do_while_loop(0);
    EXPECT_EQ(0, actual);
}

TEST(regression, do_while_loop_test_2)
{
    int actual = do_while_loop(16777216);
    EXPECT_EQ(2, actual);
}

TEST(regression, do_while_loop_test_3)
{
    int actual = do_while_loop(301115647);
    EXPECT_EQ(1, actual);
}
```
</details>

### continue, break

```cpp
int continue_break(int n) {
    int i = n, res = 0;
    do {
        res += i;
        if (res > 100) {
            break;
        }
        if (i < 20) {
            continue;
        }
    } while (false);
    return res;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, continue_break_test_1)
{
    int actual = continue_break(32);
    EXPECT_EQ(32, actual);
}

TEST(regression, continue_break_test_2)
{
    int actual = continue_break(0);
    EXPECT_EQ(0, actual);
}

TEST(regression, continue_break_test_3)
{
    int actual = continue_break(101);
    EXPECT_EQ(101, actual);
}
```
</details>

### goto

```cpp
int goto_keyword(unsigned int a) {
    unsigned int sum = 0;
    do {
        if (a == 15) {
            goto RET;
        }
        sum += a;
        a++;
    } while (a < 22);
    if (sum > 1000) {
        return 1;
    }
    return 2;
    RET: return -1;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, goto_keyword_test_1)
{
    int actual = goto_keyword(128U);
    EXPECT_EQ(2, actual);
}

TEST(regression, goto_keyword_test_2)
{
    int actual = goto_keyword(15U);
    EXPECT_EQ(-1, actual);
}

TEST(regression, goto_keyword_test_3)
{
    int actual = goto_keyword(67108883U);
    EXPECT_EQ(1, actual);
}

TEST(regression, goto_keyword_test_4)
{
    int actual = goto_keyword(12U);
    EXPECT_EQ(-1, actual);
}

TEST(regression, goto_keyword_test_5)
{
    int actual = goto_keyword(14U);
    EXPECT_EQ(-1, actual);
}
```
</details>

## Typedefs

```cpp
#include <stddef.h>

typedef size_t size_t_alias;

size_t_alias min_size_t_alias(size_t_alias a, size_t_alias b) {
    if (a < b) {
        return a;
    }
    return b;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, min_size_t_alias_test_1)
{
    // Construct input
    size_t_alias a = 0UL;
    size_t_alias b = 0UL;



    // Expected output
    unsigned long expected = 0UL;

    // Trigger the function
    unsigned long actual = min_size_t_alias(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, min_size_t_alias_test_2)
{
    // Construct input
    size_t_alias a = 0UL;
    size_t_alias b = 1UL;



    // Expected output
    unsigned long expected = 0UL;

    // Trigger the function
    unsigned long actual = min_size_t_alias(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
typedef struct __typeDefStruct {
    int a;
} TypeDefStruct2;

int sign_of_typedef_struct2(TypeDefStruct2 x) {
    if (x.a > 0) {
        return 1;
    }

    if (x.a < 0) {
        return -1;
    }

    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, sign_of_typedef_struct2_test_1)
{
    // Construct input
    TypeDefStruct2 x = {
        .a = 0};



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = sign_of_typedef_struct2(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_of_typedef_struct2_test_2)
{
    // Construct input
    TypeDefStruct2 x = {
        .a = -10};



    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = sign_of_typedef_struct2(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_of_typedef_struct2_test_3)
{
    // Construct input
    TypeDefStruct2 x = {
        .a = 1};



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = sign_of_typedef_struct2(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

```
</details>

## Storage-class specifiers
### static variables

> See comments in tests for [external variables](#extern-variables)

```cpp
static int x = 0;

static int static_simple(int dx)
{
    if (x > 0)
    {
        return x + dx;
    }
    if (x < 0)
    {
        return -x + dx;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, static_simple_test_1)
{
    // Initialize global variables
    x = 0;

    // Construct input
    int dx = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_simple(dx);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check global variables
    int expected_x = 0;
    EXPECT_EQ(expected_x, x);
}

TEST(regression, static_simple_test_2)
{
    // Initialize global variables
    x = -10;

    // Construct input
    int dx = -10;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_simple(dx);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check global variables
    int expected_x = -10;
    EXPECT_EQ(expected_x, x);
}

TEST(regression, static_simple_test_3)
{
    // Initialize global variables
    x = 1;

    // Construct input
    int dx = -1;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_simple(dx);

    // Check results
    EXPECT_EQ(expected, actual);

    // Check global variables
    int expected_x = 1;
    EXPECT_EQ(expected_x, x);
}
```
</details>

```cpp
int static_adder() {
    static int sum = 0;
    sum++;
    if (sum == 2) {
        return -1;
    }
    return sum;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, static_adder_test_1)
{
    // Construct input
    int n = 0;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = static_adder(n);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
#include <stddef.h>

static int x = 0;

static int static_simple(int dx)
{
    if (x > 0)
    {
        return x + dx;
    }
    if (x < 0)
    {
        return -x + dx;
    }
    return 0;
}

int static_adder() {
    static int sum = 0;
    sum++;
    if (sum == 2) {
        return -1;
    }
    return sum;
}

int static_adder_caller(size_t num) {
    int res = 0;
    for (size_t i = 0; i < num; ++i) {
        res = static_adder();
    }
    if (res == 1) {
        return 1;
    } else if (res == -1) {
        return 2;
    } else if (res == 3) {
        return 3;
    }
    return res;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, static_adder_caller_test_1)
{
    // Construct input
    size_t num = 0UL;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = static_adder_caller(num);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, static_adder_caller_test_2)
{
    // Construct input
    size_t num = 1UL;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = static_adder_caller(num);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, static_adder_caller_test_3)
{
    // Construct input
    size_t num = 2UL;



    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = static_adder_caller(num);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

### extern variables

```cpp
// "C.h"
extern _Bool externed_global;

// "A.c"
#include "C.h"
_Bool externed_global = 1;

// "B.c"
#include "C.h"

int use_external_simple() {
    if (externed_global) { // should always return -1
        return -1;
    }
    return 2;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, use_external_simple_test_1)
{
    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = use_external_simple();

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

> Functions depending on external state and functions, which result depends on the number of calls aren't supported by now.

```cpp
// "C.h"
extern _Bool externed_global;

// "A.c"
#include "C.h"
_Bool externed_global = 1;

// "B.c"
#include "C.h"

int use_external_simple() {
    if (externed_global) { // should always return -1
        return -1;
    }
    return 2;
}

void reset_global() {
    externed_global = 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, use_external_simple_test_1)
{



    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = use_external_simple();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, reset_global_test_1)
{



    // Expected output
    // No output variable for void function

    // Trigger the function
    reset_global();

    // Check results
    // No check results for void function
}
```
</details>


## Qualifiers
### const

```cpp
const long long * const returns_pointer_with_min_modifier(const long long a, const long long b) {
    static long long return_val;
    if (a < b) {
        return_val = a;
    } else {
        return_val = b;
    }

    return (&return_val);
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, returns_pointer_with_min_modifier_test_1)
{
    const long long actual = *returns_pointer_with_min_modifier(0LL, 0LL);
    EXPECT_EQ(0LL, actual);
}

TEST(regression, returns_pointer_with_min_modifier_test_2)
{
    const long long actual = *returns_pointer_with_min_modifier(0LL, 1LL);
    EXPECT_EQ(0LL, actual);
}
```
</details>

### volatile

```cpp
// perhaps more interesting example with many threads is needed here
const char * const foo_bar(volatile int a) {
    if (a < 0) {
        return "-1";
    } else if (a == 0) {
        return "0";
    } else {
        return "1";
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, foo_bar_test_1)
{
    const char actual = *foo_bar(2);
    EXPECT_EQ('1', actual);
}

TEST(regression, foo_bar_test_2)
{
    const char actual = *foo_bar(0);
    EXPECT_EQ('0', actual);
}

TEST(regression, foo_bar_test_3)
{
    const char actual = *foo_bar(-1);
    EXPECT_EQ('-', actual);
}
```
</details>

### restrict

```cpp
int c_strcmp_2(const  char * restrict const a, const char * restrict const b) {
    for (int i = 0; ; i++) {
        if (a[i] != b[i]) {
            return 0;
        } else {
            if (a[i] == '\0' || b[i] == '\0') {
                return a[i] == '\0' && b[i] == '\0';
            }
        }
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, c_strcmp_2_test_1)
{
    char a[] = "ccacacccc";
    char b[] = "ccacacccc";
    int actual = c_strcmp_2(a, b);
    EXPECT_EQ(1, actual);
}

TEST(regression, c_strcmp_2_test_2)
{
    char a[] = "iccccaccc";
    char b[] = "acbcbcccc";
    int actual = c_strcmp_2(a, b);
    EXPECT_EQ(0, actual);
}

TEST(regression, c_strcmp_2_test_3)
{
    char a[] = "c";
    char b[] = "c";
    int actual = c_strcmp_2(a, b);
    EXPECT_EQ(1, actual);
}

TEST(regression, c_strcmp_2_test_4)
{
    char a[] = "cbcccaccc";
    char b[] = "crbcbcccc";
    int actual = c_strcmp_2(a, b);
    EXPECT_EQ(0, actual);
}

TEST(regression, c_strcmp_2_test_5)
{
    char a[] = "";
    char b[] = "";
    int actual = c_strcmp_2(a, b);
    EXPECT_EQ(1, actual);
}
```
</details>

## compound literals

```cpp
int f() {
    struct s {
        int i;
    } *p = 0, *q;
    int j = 0;
    again:
    q = p, p = &((struct s) { j++ });
    if (j < 2) goto again;
    return p == q && q->i == 1; // always returns 1
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, f_test_1)
{
    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = f();

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

## Program execution control
### abort

```cpp
#include <stdio.h>
#include <stdlib.h>

void cleanup() {
    fprintf(stderr, "Normal program termination with cleaning up\n");
}

int call_abort() {
    if (atexit(cleanup)) {
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Going to abort the program\n");
    abort();
}
```

<details> 
  <summary>Generated tests and output</summary>

Generated tests:
```cpp
TEST(regression, cleanup_test_1)
{
    // Expected output
    // No output variable for void function

    // Trigger the function
    cleanup();

    // Check results
    // No check results for void function
}

TEST(error, call_abort_test_1)
{



    // Trigger the function
    call_abort();

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}
```

Output:
```
Going to abort the program
```
</details>

### exit

> See comments in the tests

```cpp
#include <stdio.h>
#include <stdlib.h>

void cleanup() {
    fprintf(stderr, "Normal program termination with cleaning up\n");
}

int call_exit() {
    if (atexit(cleanup)) {
        return EXIT_FAILURE;
    }
    fprintf(stderr, "Going to exit the program\n");
    exit(EXIT_SUCCESS);
}
```

<details> 
  <summary>Generated tests and output</summary>

Generated tests:
```cpp
TEST(regression, cleanup_test_1)
{
    // Expected output
    // No output variable for void function

    // Trigger the function
    cleanup();

    // Check results
    // No check results for void function
}

TEST(regression, call_exit_test_1) // should be error test
{
    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = call_exit();

    // Check results
    EXPECT_EQ(expected, actual);
}
```

Output:
```
Going to exit the program
Normal program termination with cleaning up
```
</details>

### raise

```cpp
int raise_by_num(int num) {
    return raise(num);
}

int raise_stop(int _) {
    return raise(SIGSTOP);
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
// Tests for raise_stop were not generated. Maybe the function is too complex.

TEST(regression, raise_by_num_test_1)
{
    // Construct input
    int num = 0;

    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = raise_by_num(num);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

## Error handling

### errno

```cpp
#include <errno.h>
#include <stdio.h>

int errno_usage() {
    perror("current error");
    if (errno == EACCES) {
        return -1;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, errno_usage_test_1)
{
    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = errno_usage();

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

### assert

```cpp
int buggy_function1(int a, int b) {
    if (a > b) {
        assert(a != 42);
        return a;
    }
    else {
        return b;
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, buggy_function1_test_1)
{
    // Construct input
    int a = 0;
    int b = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = buggy_function1(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, buggy_function1_test_2)
{
    // Construct input
    int a = 0;
    int b = -1;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = buggy_function1(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(error, buggy_function1_test_3)
{
    // Construct input
    int a = 42;
    int b = 0;



    // Trigger the function
    buggy_function1(a, b);

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}

```
</details>

## Dynamic memory management

> Known limitations: [[disscussion](https://github.com/UnitTestBot/UTBotCpp/issues/328#issuecomment-1190107226)]
> [Examples of usage in tests](#multidimensional-pointers)

```cpp
int reserve_and_zero_big_on_heap(size_t size) {
    if (size < 11) {
        return -2;
    }
    int *p = calloc(size + 1, sizeof(int));
    if (p == NULL) {
        return -1;
    }
    return 1;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, reserve_and_zero_big_on_heap_test_1)
{
    // Construct input
    size_t size = 2305843009213693951UL;



    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = reserve_and_zero_big_on_heap(size);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, reserve_and_zero_big_on_heap_test_2)
{
    // Construct input
    size_t size = 11UL;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = reserve_and_zero_big_on_heap(size);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, reserve_and_zero_big_on_heap_test_3)
{
    // Construct input
    size_t size = 0UL;



    // Expected output
    int expected = -2;

    // Trigger the function
    int actual = reserve_and_zero_big_on_heap(size);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(error, reserve_and_zero_big_on_heap_test_4) // incorrect suite
{
    // Construct input
    size_t size = 12UL;



    // Trigger the function
    reserve_and_zero_big_on_heap(size);

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}
```
</details>

```cpp
int out_of_bound_access_to_heap(int num) {
    int *p = calloc(5, sizeof(int));
    return p[num];
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(error, out_of_bound_access_to_heap_test_1)
{
    // Construct input
    int num = 0;



    // Trigger the function
    out_of_bound_access_to_heap(num);

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}
```
</details>

```cpp
int out_of_bound_access_to_stack(int num) {
    int a[5];
    memset(a, 0, sizeof(a));
    return a[num];
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, out_of_bound_access_to_stack_test_1)
{
    // Construct input
    int num = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = out_of_bound_access_to_stack(num);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(error, out_of_bound_access_to_stack_test_2)
{
    // Construct input
    int num = 8;



    // Trigger the function
    out_of_bound_access_to_stack(num);

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}
```
</details>

```cpp
#include <stdlib.h>
#include <string.h>

static char* leak_stack() {
    char x[524];
    memset(x, 0, sizeof(x));
    return passthrough(&x[0]);
}

char use_after_return() {
    char x = *leak_stack();
    return x;
}

int use_after_free() {
    int *p = calloc(5, sizeof(int));
    free(p);
    return *p;
}

char use_after_return() {
    char x = *leak_stack();
    return x;
}

void double_free() {
    int *p = calloc(5, sizeof(int));
    free(p);
    free(p);
}

int invalid_free() {
    char p[524];
    free(p);
    return p[0];
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(error, leak_stack_test_1) // incorrect suite
{



    // Trigger the function
    leak_stack();

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}

TEST(error, use_after_free_test_1)
{



    // Trigger the function
    use_after_free();

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}

TEST(error, use_after_return_test_1)
{



    // Trigger the function
    use_after_return();

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}

TEST(error, double_free_test_1)
{



    // Trigger the function
    double_free();

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}

TEST(error, invalid_free_test_1)
{



    // Trigger the function
    invalid_free();

    FAIL() << "Unreachable point. Function was supposed to fail, but actually completed successfully.";
}

```
</details>

## Input/Output

> `printf` and `scanf` are not supported now.

```cpp
int simple_fgetc(int x) {
    if (x >= 0 && x <= 9) {
        unsigned char a = fgetc(stdin);
        if (a >= 'a' && a <= 'z') {
            return 1;
        } else {
            return 2;
        }
    } else {
        unsigned char a = fgetc(stdin);
        unsigned char b = fgetc(stdin);
        if (a >= 'a' && a <= 'z') {
            if (b >= '0' && b <= '9') {
                return 3;
            } else {
                return 4;
            }
        } else {
            return 5;
        }
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, simple_fgetc_test_1)
{
    // Redirect stdin
    char stdin_buf[] = "d0";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = 10;



    // Expected output
    int expected = 3;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_2)
{
    // Redirect stdin
    char stdin_buf[] = "d";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = 0;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_3)
{
    // Redirect stdin
    char stdin_buf[] = "\0""\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = -1;



    // Expected output
    int expected = 5;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_4)
{
    // Redirect stdin
    char stdin_buf[] = "\xe1""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = 0;



    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_5)
{
    // Redirect stdin
    char stdin_buf[] = "d0";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = -1;



    // Expected output
    int expected = 3;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_6)
{
    // Redirect stdin
    char stdin_buf[] = "\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = 0;



    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_7)
{
    // Redirect stdin
    char stdin_buf[] = "\xe1""\xe1""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = -1;



    // Expected output
    int expected = 5;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_8)
{
    // Redirect stdin
    char stdin_buf[] = "d\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = -1;



    // Expected output
    int expected = 4;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgetc_test_9)
{
    // Redirect stdin
    char stdin_buf[] = "d@";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }
    // Construct input
    int x = -1;



    // Expected output
    int expected = 4;

    // Trigger the function
    int actual = simple_fgetc(x);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
int simple_fgets() {
    char a[8];
    fgets(a, 6, stdin);
    if (a[0] == 'u' && a[1] == 't' && a[2] == 'b' && a[3] == 'o' && a[4] == 't') {
        return 1;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, simple_fgets_test_1)
{
    // Redirect stdin
    char stdin_buf[] = "utbot";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgets_test_2)
{
    // Redirect stdin
    char stdin_buf[] = "\0""\0""\0""\0""\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgets_test_3)
{
    // Redirect stdin
    char stdin_buf[] = "\n""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgets_test_4)
{
    // Redirect stdin
    char stdin_buf[] = "utbo\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgets_test_5)
{
    // Redirect stdin
    char stdin_buf[] = "u\n""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgets_test_6)
{
    // Redirect stdin
    char stdin_buf[] = "u\0""\0""\0""\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgets_test_7)
{
    // Redirect stdin
    char stdin_buf[] = "ut\0""\0""\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, simple_fgets_test_8)
{
    // Redirect stdin
    char stdin_buf[] = "utb\0""\0""";
    int utbot_redirect_stdin_status = 0;
    utbot_redirect_stdin(stdin_buf, utbot_redirect_stdin_status);
    if (utbot_redirect_stdin_status != 0) {
        FAIL() << "Unable to redirect stdin.";
    }



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = simple_fgets();

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
int file_fgets(FILE *fA) {
  char a[8];
  fgets(a, 6, fA);
  if (a[0] == 'u' && a[1] == 't' && a[2] == 'b' && a[3] == 'o' && a[4] == 't') {
    return 1;
  }
  return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, file_fgets_test_1)
{
    write_to_file("../../../tests/lib/input_output/A", "utbo\0""");

    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "r");

    int actual = file_fgets(fA);
    EXPECT_EQ(0, actual);
}

TEST(regression, file_fgets_test_2)
{
    write_to_file("../../../tests/lib/input_output/A", "ut\n""");

    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "r");

    int actual = file_fgets(fA);
    EXPECT_EQ(0, actual);
}

TEST(regression, file_fgets_test_3)
{
    write_to_file("../../../tests/lib/input_output/A", "u\n""");

    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "r");

    int actual = file_fgets(fA);
    EXPECT_EQ(0, actual);
}

TEST(regression, file_fgets_test_4)
{
    write_to_file("../../../tests/lib/input_output/A", "\n""");

    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "r");

    int actual = file_fgets(fA);
    EXPECT_EQ(0, actual);
}

TEST(regression, file_fgets_test_5)
{
    write_to_file("../../../tests/lib/input_output/A", "utb\n""");

    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "r");

    int actual = file_fgets(fA);
    EXPECT_EQ(0, actual);
}

TEST(regression, file_fgets_test_6)
{
    write_to_file("../../../tests/lib/input_output/A", "utbot");

    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "r");

    int actual = file_fgets(fA);
    EXPECT_EQ(1, actual);
}
```
</details>

```cpp
char file_fwrite(FILE *fA, int x) {
  if (x > 0) {
    char a[] = "Positive";
    fwrite(a, sizeof(char), 8, fA);
    return 'P';
  } else if (x < 0) {
    char a[] = "Negative";
    fwrite(a, sizeof(char), 8, fA);
    return 'N';
  } else {
    char a[] = "Zero";
    fwrite(a, sizeof(char), 4, fA);
    return 'Z';
  }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, file_fwrite_test_1)
{
    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "w");

    char actual = file_fwrite(fA, -10);
    EXPECT_EQ('N', actual);
}

TEST(regression, file_fwrite_test_2)
{
    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "w");

    char actual = file_fwrite(fA, 0);
    EXPECT_EQ('Z', actual);
}

TEST(regression, file_fwrite_test_3)
{
    struct _IO_FILE * fA = (UTBot::FILE *) fopen("../../../tests/lib/input_output/A", "w");

    char actual = file_fwrite(fA, 1);
    EXPECT_EQ('P', actual);
}
```
</details>

### Algorithms

> Simple functions like `abs`, `atoi` are supported, but complex functions like `qsort` only partially supported.

```cpp
#include <stdlib.h>

int abs_val(int x) {
    if (x < 0 && abs(x) > 0) {
        return -1;
    } else if (x > 0) {
        return 1;
    }
    return abs(x);
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, abs_val_test_1)
{
    // Construct input
    int x = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = abs_val(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, abs_val_test_2)
{
    // Construct input
    int x = 2;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = abs_val(x);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, abs_val_test_3)
{
    // Construct input
    int x = -1;



    // Expected output
    int expected = -1;

    // Trigger the function
    int actual = abs_val(x);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
#include <stdlib.h>

int sign(char const *str) {
    int x = atoi(str);
    if (x == 5) {
        return 7;
    }
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, sign_test_1)
{
    // Construct input
    char str_buffer[] = "00000005@";
    const char * str = str_buffer;



    // Expected output
    int expected = 7;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_2)
{
    // Construct input
    char str_buffer[] = "000000005";
    const char * str = str_buffer;



    // Expected output
    int expected = 7;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_3)
{
    // Construct input
    char str_buffer[] = "";
    const char * str = str_buffer;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_4)
{
    // Construct input
    char str_buffer[] = "5@ccccccc";
    const char * str = str_buffer;



    // Expected output
    int expected = 7;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_5)
{
    // Construct input
    char str_buffer[] = "22222222@";
    const char * str = str_buffer;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_6)
{
    // Construct input
    char str_buffer[] = "2@ccccccc";
    const char * str = str_buffer;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_7)
{
    // Construct input
    char str_buffer[] = "{cccccccc";
    const char * str = str_buffer;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_8)
{
    // Construct input
    char str_buffer[] = "hcccccccc";
    const char * str = str_buffer;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, sign_test_9)
{
    // Construct input
    char str_buffer[] = "Ycccccccc";
    const char * str = str_buffer;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = sign(str);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

## Pseudo random number generation

```cpp
#include <stdlib.h>

int randdd() {
    srand(42);
    return rand();
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, randdd_test_1)
{



    // Expected output
    int expected = 71876166;

    // Trigger the function
    int actual = randdd();

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
#include <stdlib.h>
#include <time.h>

int random_random() {
    srand(time(NULL));
    return rand();
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, random_random_test_1) // fails
{



    // Expected output
    int expected = 1791780077;

    // Trigger the function
    int actual = random_random();

    // Check results
    EXPECT_EQ(expected, actual);
}

```
</details>

```cpp
#include <stdlib.h>

int sign_rand() {
    srand(42);
    int x = rand();
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, sign_rand_test_1)
{



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = sign_rand();

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

```cpp
#include <stdlib.h>

int sign_rand_seeded(unsigned seed) {
    srand(seed);
    int x = rand();
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, sign_rand_seeded_test_1)
{
    // Construct input
    unsigned int seed = 0U;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = sign_rand_seeded(seed);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>


```cpp
#include <stdlib.h>
#include <time.h>

int sign_rand_rand() {
    srand(time(NULL));
    int x = rand();
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    }
    return 0;
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, sign_rand_rand_test_1)
{



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = sign_rand_rand();

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

## Complex number arithmetic

```cpp
#include <complex.h>
#include <tgmath.h>

double complex square_i() {
    return I * I;     // imaginary unit squared
}
```

<details> 
  <summary>Generated tests</summary>

No tests generated. Couldn't find any supported methods. Please check if source directories are specified correctly. See logs for more details about unsupported functions.

LOGS: Function 'square_i' was skipped, as return type '_Complex double' is not fully supported: Type is unknown

</details>

## Generic selection

```cpp
#define typeid(x) _Generic((x), \
        _Bool: 0,               \
         char: 1,               \
          int: 2,               \
        float: 3,               \
      default: 4)

int get_typeid(unsigned short int casen) {
    switch (casen) {
        case 0:
            return typeid((_Bool const) 0);
        case 1:
            return typeid((char) 'c');
        case 2:
            return typeid(24);
        case 3:
            return typeid(42.f);
        default:
            return typeid("char const *");
    }
}
```

<details> 
  <summary>Generated tests</summary>

```cpp
TEST(regression, get_typeid_test_1)
{
    // Construct input
    unsigned short casen = 4;



    // Expected output
    int expected = 4;

    // Trigger the function
    int actual = get_typeid(casen);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, get_typeid_test_2)
{
    // Construct input
    unsigned short casen = 0;



    // Expected output
    int expected = 0;

    // Trigger the function
    int actual = get_typeid(casen);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, get_typeid_test_3)
{
    // Construct input
    unsigned short casen = 3;



    // Expected output
    int expected = 3;

    // Trigger the function
    int actual = get_typeid(casen);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, get_typeid_test_4)
{
    // Construct input
    unsigned short casen = 2;



    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = get_typeid(casen);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, get_typeid_test_5)
{
    // Construct input
    unsigned short casen = 1;



    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = get_typeid(casen);

    // Check results
    EXPECT_EQ(expected, actual);
}
```
</details>

***

### Example template

```cpp
// Enter the source code
```

<details> 
  <summary>Generated tests</summary>

```cpp
// Enter generated tests
```
</details>


