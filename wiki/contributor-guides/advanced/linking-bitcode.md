<!---
name: Linking bitcode
route: /docs/cpp/advanced/linking-bitcode
parent: Documentation
menu: Advanced
description: Describe how UTBotCpp linking bitcode
--->

# Linking bitcode

Let us name a C/C++ type, struct or a function definition as a code symbol. If any C/C++ code uses a symbol, for the
code to be compiled, linked and run, the code artifact must contain that symbol.

In the example, max function does not use any other symbols in the code. Thus, to symbolically execute max function and
generate tests, it is sufficient for UTBot to compile the corresponding KLEE file into LLVM IR and pass the bitcode to
symbolic execution engine.

However, in real projects, functions call other functions, and the common case in C/C++ is that symbols may be defined
in another object file, which has to be linked with the original function object file. Hence, LLVM bitcode of KLEE files
needs to be linked with other bitcode files, which contain needed symbols.

As it was mentioned, UTBot can get information about user project build structure by running Bear tool.

![c-example project](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/targetExample.png)

Figure above presents the c-example project which will be referenced to illustrate UTBot linking behaviour. Suppose that
UTBot is requested to perform test generation for a function inside file3.c. Prior to linking phase, UTBot receives
information about what object file file3.c compiles into (in the figure, this file is called file3.o), as well as the
information about project linkage structure. Note that link_commands.json presented
in [Compile database](compile-database) is exactly the json UTBot generates for c-example project. During test
generation process, it is stored in a graph like the one in the figure. A C/C++ project consists of object files, static
and dynamic libraries, and executables. Let us call a static library, a dynamic library, or an executable a project
target.

If functions from file3.o use symbols from other object files in the project, it is needed to link file3.o with them to
provide complete function definitions for KLEE. UTBot tries to minimally change user code and test it as-is, so it is
desired to link code in the same way as it is linked in a user project. UTBot uses a reasonable assumption: If user
project defines project targets, which include an object file, they are likely to contain all the needed symbols for it.
This is not always true, but it is conceivable that at least one project target actually uses functions from an object
file, so it should have all the needed symbols. Under this assumption, UTBot linkage algorithm looks for a suitable
project target that includes the selected object file and then creates LLVM bitcode module of this target for symbolic
execution.

Linking a project target. To link a project target, UTBot first links its dependencies recursively, and then puts them
together into the project target, operating in accordance with the following rules:

* If currently processed project unit is an object file, it is built into LLVM IR from corresponding sources using clang
  compiler with -emit-llvm option; if tests generation was requested for this file, then not the original file is
  compiled, but the KLEE file for it.
* If currently processed project unit is a static library, it is linked from LLVM bitcode files using ar tool with
  –plugin LLVMGold.so. This stage uses the [LLVM Gold plugin](https://llvm.org/docs/GoldPlugin.html), as it allows archived result to be reused in another
  link action, whereas default ar behaviour forbids that.
* If currently processed project unit is a shared library or an executable, it is linked from its dependencies using ld
  and LLVM Gold plugin, as shared libraries also can be linked into one another.

After linkage, UTBot will have a module with a KLEE entry function wrapper for the code and all symbols, which the
project function might call. Such a module can be passed to KLEE. Suppose that test generation is requested for
functions inside file3.c. Then UTBot will traverse c-example project as presented in figure below.

![linking scheme](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/linkExample.png)

There are more problems that make the linking process more complex. Link process of exe1 target from c-example is
addressed to illustrate the algorithm.

There might be a case in which no symbol from static library (for instance, lib.a) is used in shared library (exe1),
which appears to be a selected target. This is possible, if the dependency of exe1 from lib.a is somehow mentioned in
the project build system, but no code from lib.a is actually used in exe1. ld plugin detects this and does not include
the lib.bc bitcode in the final module. Thus, the module will not have a symbol for KLEE entry wrapper, which UTBot has
linked into lib.bc, and, consequently, no tests will be generated as KLEE will not have a function definition to
interpret.

This can be solved by adding –whole-archive option to link command, which forces all the symbols to be included, even if
they are unused. But, if this approach is used, another problem occurs. lib.a may contain a function, which has a
complete definition only when linked with exe2. Consider a function 𝑓 from lib.a, which uses function 𝑔 from file4.o.
If lib.a is linked with exe1, 𝑓 is discarded by the linker, but if its inclusion is forced by –whole-archive, the
linkage will fail because there is no symbol for 𝑔 both in lib.a and exe1.

Given that, forcing symbols into final module is not a good idea in general, and it is better to try another target; but
if no project target uses symbols from lib.a, then this way of linking may be applied. If user project successfully
links, UTBot is able to rebuild it in LLVM IR and substitute any project function with a symbolic wrapper.

Choosing the right project target. To sum up, for correct linking of a bitcode module UTBot needs to find a project
target which uses symbols from a specific object file. There are two types of targets UTBot tries to iteratively select
and link:

* Direct parents of the object file in link project structure. Typically, they are static libraries, which contain
  needed symbols, because ar tool, with which they are built, includes all symbols from object files;
* Targets, which recursively include the request file and do not have any parents in linking structure — root project`
  targets. UTBot considers them for linking on the grounds that if there are dependencies between source files,
  unmentioned in build system, then chances are those dependencies will not produce a problem if we build "the biggest"
  project targets, which are most likely to be used in reality.

UTBot finds mentioned targets by an object file and tries to iteratively build them into LLVM IR. When a correctly built
target is found, linking process stops and the final bitcode is passed to KLEE.

Although UTBot tries to guess project target automatically and it works for most users, in some scenarios it is
impossible to guess what the user actually wants. For example, function can have multiple behaviour depending on what it
is linked with. In this case, UTBot offers its users a possibility to manually specify the project target.