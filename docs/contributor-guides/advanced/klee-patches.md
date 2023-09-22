<!---
name: KLEE patches
route: /docs/cpp/advanced/klee-patches
parent: Documentation
menu: Advanced
description: List of KLEE patches by UTBot team
--->

# KLEE

UTBot uses [KLEE](https://github.com/klee/klee/releases/tag/v2.2)
as backend for test generation. KLEE was chosen as UTBot backend because it is able to operate on almost any C/C++
code (translated to LLVM IR). KLEE holds its place of one of the top tools for software verification and test generation
in [recent years](https://doi.org/10.1007/s10009-020-00570-3), remains in active development and improvement stage, has
a wide community and is highly customizable for one’s needs. While the main goal of UTBot is to make tests generation
techniques accessible for regular developer, additional goal to improve default KLEE coverage is also persuaded. For
this goal, KLEE was patched and updated in various ways.

Expanding the supported set of LLVM IR. KLEE 2.2 support a wide set of LLVM IR, which C/C++sources compile to. However,
there were unsupported instructions, which were determined to be vital for UTBot, so an effort was made to support them
in KLEE.

## Floating point

Namely, floating-point and sanitizer instructions were supported for symbolic execution. For floating-point
instructions,
[KLEE-Float](https://doi.org/10.1109/ASE.2017.8115670), a development, which addresses the same problem, was rebased on
KLEE 2.2; also, a floating-point runtime library was added. For sanitizer instructions, corresponding handlers were
added. Those two patches are important for UTBot: symbolic execution of floating-point instructions increases code
coverage, while sanitizer instructions were needed as UTBot rebuilds a given project with sanitizer support to increase
the quality of software verification. Sanitizer introduces new branches for many corner cases (like buffer overflows)
and tests are generated to cover these interesting cases.

## Lazy initialization

KLEE symbolically executes functions one at a time, and if the code is symbolically executed in an empty symbolic state,
some paths may be left unexplored because there are no objects in the heap. To overcome this obstacle, the lazy
initialization technique has been implemented. The memory operations
(reading, writing, copying and freeing memory) in KLEE are implemented in the following way: to execute the memory
operation on pointer 𝑝, the engine checks for every heap location if the pointer can reference this location. For every
satisfiable variant, it forks the symbolic state adding into the path condition the equality of 𝑝 and the heap address
under consideration. This behaviour has been extended. Now, the engine checks if current heap locations do not cover
every possible variant 𝑝 can reference. If not, then the engine instantiates the new block of symbolic memory and forks
the state adding into the path condition the equality of 𝑝 and the concrete heap address of this fresh block.

## KLEE parameters

A parameter bcov_check_interval was added to KLEE; it is designed to put timeout on symbolic execution. When KLEE
analyses a branch for a while and does not increase code coverage for specified amount of time, the symbolic execution
for such a branch is stopped.

## POSIX Runtime

UTBot is able to generate tests for functions, that use I/O processing via stdin and stdout — a way of substituting them
with custom data in test bodies was invented. KLEE has its POSIX runtime, which allows symbolic execution of code that
uses I/O calls (like open() or fread()). Essentially, the runtime initializes a symbolic array for each file descriptor
and redirects I/O calls to wrapper functions working with such variables. These wrapper functions can be treated as
stubs for POSIX functions calls, but provided not by UTBot, but by KLEE itself. UTBot only adds variables to KLEE POSIX
runtime, which stand for the number of bytes used in symbolic stdin, stdout and files. Consequently, UTBot will have the
information on exact number of bytes used by user code in every execution path and will not pollute test bodies with
excessive data created by KLEE.

## Interactive mode

In the previous utbot's version `Generate Tests For Project` and `Generate Tests For Current File` launch `klee` for
every function in a project or a file respectively. `Klee` spends significant part of time to preprocess an input file,
which small changes, when we generate tests for all functions in one file. Only entrypoint changes between different
launches of `klee`.

`Interactive mode` solves this problem: now we generate tests for all functions in a file for one launch of `klee`. And
we make several parallel processes with `fork()` after `klee` finishes preprocessing to work with test generation for
different functions.

See the difference in time:

| interactive_mode \ number of functions in file  | 10    | 50    | 100    | 500    |
|-------------------------------------------------|-------|-------|--------|--------|
| without interactive                             | 14.2s | 71.6s | 143.9s | 963s   |
| with interactive and 1 parallel process         | 3.8s  | 14.9s | 30.3s  | 189s   |
| with interactive and 5 parallel process         | 2.2s  | 6.6s  | 12.4s  | 66.8s  |
| with interactive and 10 parallel process        | 1.9s  | 5.7s  | 10.4s  | 52.3s  |

## Inline assembly

Calling inline assembly as _external_ functions has been added. Now `klee` can execute branches with inline assembly
and `UTBot` generate tests for it. Due to the fact that there is an _external_ call, we may not cover all branches,
but `UTBot` will generate at least some tests. `klee` can call the assembler of the platform on which it itself runs. In particular,
vector instructions work on `x86`. `klee` can execute all assembly that represent as `call <type> asm ...` in `LLVM IR`,
so `UTBot` still can't do anything with assembler files. Also, it does not work with the assembler at the module level, in particular, assembly functions do not work:
```c
int my_sub(void); //call my_sub will fail

asm (
"my_sub:\n\t"
"  mov $100,%eax\n\t"
"  ret\n\t"
);
```