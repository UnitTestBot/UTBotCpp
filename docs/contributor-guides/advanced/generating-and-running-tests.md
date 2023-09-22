<!---
name: Generating and running tests
route: /docs/cpp/advanced/generating-and-running-tests
parent: Documentation
menu: Advanced
description: Describe how UTBotCpp generate and run tests
--->

# Generating and running tests

After KLEE run, UTBot obtains test cases for each project function
previously specified as an entry point. Listing bellow shows an example
of generated test for max function.

```c++
#include "basic_functions_test.h"

#include "gtest/gtest.h"
namespace UTBot {
TEST(regression, max_test_1)
{
    // Construct input
    int a = 0;
    int b = -1;
    // Expected output
    int expected = 0;
    // Trigger the function
    int actual = max(a, b);
    // Check results
    EXPECT_EQ(expected, actual);
}
```

Function arguments and return value, previously marked as
symbolic, are printed out to C++ test file, using GoogleTest framework.
UTBot chose this format, because it is popular in reality and
can be applied to both C and C++ user code. For each test file, a
corresponding header file is generated; its contents are explained
below.

UTBot performs a complex transformation of KLEE output into
tests sources. KLEE returns raw byte values for symbolic objects,
and UTBot is able to construct human-like code that assigns these
bytes to variables. For instance, UTBot sets structures fields one by
one with assignment operators instead of performing one memcpy().

UTBot provides its users an option to compile test files, link them
with user code and run them to get test results and coverage rates.
UTBot Makefile that contains compilation and run commands is
generated along with tests based on link_commands.json. There
are many complications arising when UTBot links user code to
generated tests:

 * If user code contains a C static function, which was analysed
by UTBot, generated tests binary can not be linked, because
static functions are available only in the source file they are
declared. To handle this problem, UTBot generates wrapper
files with non-static function that include original files via
`#include directory`. Functions in wrapper files have same
signatures as original static functions and wrapper functions
delegate to original static functions in their bodies. For tests
to be compiled and run, UTBot substitute wrapper files instead of original files in UTBot Makefile via environment
variables. Functions in wrapper files and user files have different names, so a name mangling technique is applied to
allow using user-defined names for them in test files.
 * In case of using stubs, tests are generated and run with original files in UTBot Makefile replaced by stubs via environment
variable.
 * C++ has some keywords, which are absent in C, for example,
try or reinterpret_cast. Those keywords can be used as
function/variable names in C, so, in test headers, UTBot
guards declared user function wrappers with a #define and
an #undef macro, renaming them in a way they do not clash
with any C++ specific keyword.

![Creating test executable on c-example project](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/runExample.png)

Figure 7 represents the way of creating test executable for a generated test file. UTBot reuses the structure it creates when linking
project files, takes the same compile and link commands, but this
time user files are compiled not to LLVM bitcode, but to native binaries. UTBot takes the same project target it used to pass to KLEE
and links it with generated test file. This way, functions called from
the test file have all symbols they need. One important step is that,
during this stage, UTBot links user executables as shared libraries
for them to be linkable with generated test files. Despite UTBot
uses only Clang to compile user project into LLVM IR and to obtain
compile_commands.json, it is possible run tests with a broader
set of compilers, with UTBot supporting gcc, g++, clang and
clang++.