<!---
name: How UTBot Works
route: /docs/cpp/advanced/utbot-inside
parent: Documentation
menu: Advanced
description: UnitTestBot C pipeline is described in this section. It gives a basic overview of the product inside that might be useful for the ones who would like to contribute to UnitTestBot.
--->

# How UTBot Works

UTBot uses KLEE symbolic execution engine to obtain information about program branches. C/C++ projects usually consist
of source files and headers, which are compiled and linked into libraries and executable files (while libraries can also
be linked to one another or other executables, forming a directed acyclic graph of project linkage). Executables and
libraries can be considered as the artifacts of project building. KLEE operates with [LLVM IR](???) so, to symbolically
execute code from a given product, UTBot needs the information about how the project is compiled and linked in order to
be able to recompile it into LLVM IR. UTBot is able to get this information from project build system via Bear
tool [[more info](compile-databasse)]. UTBot supports CMake and Makefile build systems.

Then, UTBot analyses given sources using [clang-tooling](https://clang.llvm.org/docs/HowToSetupToolingForLLVM.html) to
obtain C/C++ AST of files under test. Note that clang-tooling requires a compilation database to work. Essentially, a
compilation database is a structural representation of compile commands used to obtain C/C++ object files from project
sources, and it is also retrieved from the build system by UTBot. UTBot treats functions as structural units of the
project and generates tests for them. In order to achieve this, UTBot creates symbolic wrappers called KLEE files for
every project source file [[more info](preparing-sources-for-klee)], that use symbolic variables concept for symbolic
execution. Symbolic variables can be treated as logical constraints for variables, which are transformed into concrete
values as the result of symbolic execution. Symbolic variables are the core of symbolic execution approach to software
verification, and they are used in many successful tools for software verification and test generation.

Then, UTBot uses information about project compilation and linkage to rebuild the project in LLVM
bitcode [[more info](linking-bitcode)]. This is done to pass the linkage result (.bc file) to KLEE. If a function calls
other functions from the project, UTBot can provide KLEE with their bitcode as well, or, depending on user preferences,
use generated symbolic stubs for them [stubs](stubs).

KLEE produces files that represent test cases for a given function, and UTBot parses and transforms them to C++ test
files (for supported C/C++ features, see [C syntax](c-syntax), [C++ syntax](../../cpp-syntax.md)), which present the test cases
with [GoogleTest](https://google.github.io/googletest/) framework. UTBot also provides its users with a build system for
generated tests, automatically compiling and linking them with project files. Users can run
tests [[more info](generating-and-running-tests)] with a single click and see the results and coverage information in
VSCode Panel. UTBot uses Makefiles for test runs [[more info](incrementality)], so UTBot tests can be used in continuous
integration.

![UTBotCpp pipeline](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/pipelineUtbot.png)

The most important things are described below in sequential order.

## Tests Generation

1. [Client](install-vscode-plugin) sends request (one of [types](generating-tests))
   to [server](install-server).
   The [corresponding](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/testgens) object aggregates
   requests' information.
2. [Fetcher](https://github.com/UnitTestBot/UTBotCpp/blob/main/server/src/fetchers/Fetcher.cpp) fetches types, methods,
   global variable usages, array usages, includes for all files in request. It
   uses [LibTooling](https://clang.llvm.org/docs/LibTooling.html) for parsing source code, traversing types, etc.
3. [SourceToHeaderRewriter](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/clang-utils/SourceToHeaderRewriter.cpp)
   generates headers (.h files) based on source (.c files).
4. [FeaturesFilter](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/FeaturesFilter.cpp) filters out all
   methods except the ones listed in [supported syntax](c-syntax).
5. [Synchronizer](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/Synchronizer.cpp)
   synchronizes [stubs](stubs) and wrappers for all files
   in [compile_commands.json](https://clang.llvm.org/docs/JSONCompilationDatabase.html).
6. [KleeGenerator](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/KleeGenerator.cpp) generates klee files
   as well as source files, also builds them in [bitcode](https://llvm.org/docs/BitCodeFormat.html) files.
7. [Linker](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/building/Linker.cpp) links bitcode files
   together into module.
8. [Linker](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/building/Linker.cpp)
   writes [Makefiles](makefiles).
9. [KleeRunner](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/KleeRunner.cpp)
   runs [KLEE](https://klee.github.io/) on module consecutively for each method from request.
10. [KleeGenerator](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/KleeGenerator.cpp) parses output of
    KLEE and writes files of [GoogleTest](https://github.com/google/googletest).
11. [Server](install-server) sends final response to client and also transfers generated files for remote scenario.

## Tests Running

When user runs the tests, server performs various operations that might depend on the compiler used by the project.

1. [Client](install-vscode-plugin) sends request to [server](install-server).
2. [TestRunner](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/TestRunner.cpp) chooses coverage
   tool based on the compiler used by the project. It
   is [GcovCoverageTool](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/GcovCoverageTool.cpp) in
   case of [gcc](https://gcc.gnu.org/)
   and [LlvmCoverageTool](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/LlvmCoverageTool.cpp)
   in case of [clang](https://clang.llvm.org/).
3. [TestRunner](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/TestRunner.cpp) collects list of
   tuples (file path, suite name, test name) based on the information from the [client](install-vscode-plugin).
4. [TestRunner](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/TestRunner.cpp) incrementally
   builds tests with instrumented options (coverage and sanitizer). To achieve this [Makefiles](makefiles) are used.
5. [TestRunner](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/TestRunner.cpp) runs tests by
   calling [make](https://www.gnu.org/software/make/manual/make.html) on makefiles that correspond to the given sources.
6. [CoverageAndResultsGenerator](https://github.com/UnitTestBot/UTBotCpp/blob/main/server/src/coverage/CoverageAndResultsGenerator.cpp)
   collects coverage information using previously chosen tool.
    - [GcovCoverageTool](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/GcovCoverageTool.cpp)
      runs [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)
      on [.gcno and .gcda](https://gcc.gnu.org/onlinedocs/gcc/Gcov-Data-Files.html) files. It provides a report
      in `.json` format.
    - [LlvmCoverageTool](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/LlvmCoverageTool.cpp)
      runs [llvm-profdata](https://llvm.org/docs/CommandGuide/llvm-profdata.html) to merge several .profraw files into
      one. It provides a report in `.json` format using [llvm-cov](https://llvm.org/docs/CommandGuide/llvm-cov.html).
7. Both [GcovCoverageTool](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/GcovCoverageTool.cpp)
   and [LlvmCoverageTool](https://github.com/UnitTestBot/UTBotCpp/tree/main/server/src/coverage/LlvmCoverageTool.cpp)
   read generated `.json` file and determine set of covered lines.
8. Finally, [Server](install-server) sends response to the client.
