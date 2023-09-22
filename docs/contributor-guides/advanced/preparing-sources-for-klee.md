<!---
name: Preparing sources for KLEE
route: /docs/cpp/advanced/preparing-sources-for-klee
parent: Documentation
menu: Advanced
description: Describe how UTBotCpp prepare source files for KLEE
--->

KLEE has a simple API of creating symbolic variables and adding constraints to them; it is, in fact, that simple that
the process of applying the API to source code can be easily automated. UTBot considers functions as structural units
for its process of generating tests. Firstly, when a project is passed to UTBot, it traverses project sources ASTs using
clang-tooling and detects all function declarations. For each declaration, types and names of its parameters are
memorized along with its return type. Let us consider a simple function:

```c++
int max( int a , int b ) {
    if ( a > b ) {
        return a ;
    } else {
        return b ;
    }
}
```

Treating the function as a unit for generating tests, UTBot marks its arguments and result as symbolic variables,
forcing KLEE to calculate input parameters as well as the expected value. This is done by generating the wrapper
klee_entry__<function_name>
for every function under test in so called KLEE file.

```c++
#include "basic_functions.c"
#include "klee/klee.h"
#include <stdlib.h>
int klee_entry__max (int argc, char **argv, char **envp ) {
    int a;
    klee_make_symbolic (&a, sizeof(a) , "a");
    klee_prefer_cex (&a , a >= −10 & a <= 10 );
    int b;
    klee_make_symbolic (&b, sizeof(b) , "b");
    klee_prefer_cex (&b , b >= −10 & b <= 10 );
    int res;
    klee_make_symbolic (&res, sizeof(res), "res");
    int utbot_tmp = max_(a, b);
    klee_assume (utbot_tmp == result);
    return 0 ;
}
```

Let us break the wrapper down and explain its contents in detail:

* Initial source file is included as "basic_functions.c" file; As a result of doing this, the function and its wrapper
  are provided with the same set of defined symbols, which means that UTBot can call the tested function from the
  wrapper file and with the same behaviour;
* "klee/klee.h" is included to provide the file with KLEE API.
* A KLEE entry function wrapper is created for targeted function. Inside it, the following things happen:
    - Function arguments are marked as symbolic. In simplest case, this is achieved by calling klee_make_symbolic on a
      fresh variable address. If the argument type is pointer, UTBot performs a more sophisticated approach — creates a
      symbolic buffer and then creates pointer on stack that references this buffer. If the argument is a string, UTBot
      automatically adds \0 symbol to the end of symbolic buffer for it to represent a C-style string.
    - Soft constraints are imposed on symbolic variables, advising KLEE to provide more human-readable test cases. Those
      constraints are applied with a klee_prefer_cex call. Soft constraints are desirable but not necessary so KLEE can
      discard them if needed for a branch exploration. For symbolic arrays, soft constraint is applied for every item.
    - A symbolic variable is created for a function result. That is done in the same way as creating symbolic variables
      for function arguments.
    - A concrete variable is created and its value is assigned to a function call result. When the file will be passed
      into KLEE, it is this line in which the symbolic engine will branch and explore target function paths.
    - A constraint is applied to new symbolic variable, setting it equal to calculated function result. This is done
      with klee_assume call. Since KLEE writes out all symbolic variables, UTBot forces function result to be calculated
      for every argument set KLEE has explored. Therefore, UTBot does not need to rerun user code for received argument
      sets and already has the information about function return value.

In advanced generation scenario when UTBot generates test that run through specified line of code, UTBot modifies source
code of function under test (virtually, by clang tooling), adding global variable line_touched; and statement
line_touched=1; next to the selected line of code. Change of this new global variable can be monitored by klee_assume
similarly to assuming the function result value.