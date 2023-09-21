<!---
name: Stubs
route: /docs/cpp/usage/stubs
parent: Documentation
menu: Usage
description: Stubs are an important aspect of UnitTestBot that allows to replace calls to some functions.
--->

# Stubs

Here we describe how stubs are generated and what they are needed for.

> 📝**Note**
>
> **Stub** is a function which emulates behavior of another function it is written for.
> It's often useful to write stubs for functions from another module that you don't want to test.

### When stubs are used

UTBot generates stubs for every function in the project first time you open it and synchronize each time before test
generation.

First time you open the project you will see `UTBot is generating stubs for project...` message. It means that for each
file in your project UTBot generates file which contains all non-static functions signatures from the source file. Stubs
can be used as substitutes for calling functions from another module. Consider the following project structure:

```
executable
|---- lib1.a
|     |---- A.c
|     +---- B.c
|---- lib2.a
|     |---- C.c
```

If we run test generation for `C.c`, all functions from `lib1.a` will be replaced with their stubs on build step. So, if
the function `foo` from `C.c` calls the function `bar` from `A.c`, then stub for the function `bar` will be invoked
instead. Note that you can still generate tests using the exact definition of `bar`; for that, you will need to
unmark `Use Stubs` checkbox in [settings](vscode-extension-settings#use-stubs).

### Stub file example

Stub files can be found in `tests/stubs` folder. This is an example of stub file generated
for `c-example/lib/dependent_functions.c` from
the [example project](https://github.com/UnitTestBot/UTBotCpp/tree/main/integration-tests/c-example):

```cpp
// 1624618650000000000
// Please, do not change the line above

#ifdef
KLEE_MODE
extern void klee_make_symbolic(void *addr, unsigned long long nbytes, const char *name);
#endif
#include
"dependent_functions_stub.h"

#define
NULL ((void*)0)

int double_max_symbolic;
int double_max(int a, int b) {
static int firstTimeCall = 1;
#ifdef
KLEE_MODE
if (firstTimeCall == 1) {
firstTimeCall = 0;
klee_make_symbolic(&double_max_symbolic, sizeof(double_max_symbolic), "double_max_symbolic");
}
#endif
return double_max_symbolic;
}

```

The first line contains timestamp of stub file creation. It is required for synchronization with the source code, so,
please, do not modify this line:

```cpp
// 1619438023000000000
```

Then, if a `KLEE_MODE` macro is passed during preprocessing, we add a KLEE declaration to allow us to use symbolic
return value. `KLEE_MODE` serves for both test generation and test running purposes.

```cpp
#ifdef KLEE_MODE
extern void klee_make_symbolic(void *addr, unsigned long long nbytes, const char *name);
#endif
```

This variable stores return value of the stub function:

```cpp
int double_max_symbolic;
```

Stub function signature is always the same as the signature of the source function. If it's not (for example, if the
function was modified), then UTBot will synchronize them by rewriting stub function with the new one:

```cpp
int double_max(int a, int b)
```

If UTBot uses stubs, then `KLEE_MODE` is defined, and the return value is made symbolic. This way KLEE can decide what
values to return to satisfy the execution paths:

```cpp
static int firstTimeCall = 1;
#ifdef KLEE_MODE
    if (firstTimeCall == 1) {
        firstTimeCall = 0;
        klee_make_symbolic(&double_max_symbolic, sizeof(double_max_symbolic), "double_max_symbolic");
    }
#endif
```

### Stub headers

For each stub, a header is generated. It contains definitions of types and structures used in function headers, allowing
stubs to be compilable. Stub headers may contain definitions fetched from system headers and may look obscure, but
usually you will not modify them.

> 📝**Note**
>
> You can change the stub function body however you want. UTBot saves custom code inside the function while synchronizing. However, if the source function signature changes, then stub function will be completely rewritten.

### Tests with stubs

For the tests that use stubs, the return values are generated in a such way that the code coverage is maximized. For
example:

```cpp
extern "C" char pointerToPointer_symbolic;

TEST(regression, calc_two_numbers_f_test_1)
{
    // Initialize symbolic stubs
    pointerToPointer_symbolic = '\x10';

    // Construct input
    char a = 'c';
    char b = 'p';

    // Expected output
    int expected = 2;

    // Trigger the function
    int actual = calc_two_numbers_f(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}

TEST(regression, calc_two_numbers_f_test_2)
{
    // Initialize symbolic stubs
    pointerToPointer_symbolic = 'b';

    // Construct input
    char a = 'c';
    char b = 'b';

    // Expected output
    int expected = 1;

    // Trigger the function
    int actual = calc_two_numbers_f(a, b);

    // Check results
    EXPECT_EQ(expected, actual);
}
```

Here `pointerToPointer_symbolic` stores return value for `pointerToPointer` stub function.

### Modifying stubs

As it was noted, it is easy to rewrite UTBot stubs into anything you want them to be. You should modify only function
bodies and not touch header include, or `KLEE_MODE` ifdefs. Consider the simplest example:

```cpp
// 1624618650000000000
// Please, do not change the line above

#ifdef
KLEE_MODE
extern void klee_make_symbolic(void *addr, unsigned long long nbytes, const char *name);
#endif
#include
"dependent_functions_stub.h"

#define
NULL ((void*)0)

int double_max_symbolic;
int double_max(int a, int b) {
return a;
}
```

In such a case, UTBot will generate tests for functions from other CMake modules implying
that `double_max(int a, int b) = a`.

However, you can also use symbolic variable power when modifying stubs. Suppose that you want your function not to
return any value, but perform some checks on the arguments prior to that. Also, there may be a special case in that
function that is highly important to be reflected in the stub. In that case, you can insert those checks in the stub,
and, if they succeed, return a symbolic value:

```cpp
// 1624618650000000000
// Please, do not change the line above

#ifdef
KLEE_MODE
extern void klee_make_symbolic(void *addr, unsigned long long nbytes, const char *name);
#endif
#include
"dependent_functions_stub.h"

#define
NULL ((void*)0)

int double_max_symbolic;
int double_max(int a, int b) {
if (a == 100 && a > b) {
return 100;
}
static int firstTimeCall = 1;
#ifdef
KLEE_MODE
if (firstTimeCall == 1) {
firstTimeCall = 0;
klee_make_symbolic(&double_max_symbolic, sizeof(double_max_symbolic), "double_max_symbolic");
}
#endif
return double_max_symbolic;
}
```

By this change, `double_max(a, b)` will preserve its behaviour if a certain condition holds. This principle can be used
to achieve the similarity of the stub and the original code while leaving out big parts of code.