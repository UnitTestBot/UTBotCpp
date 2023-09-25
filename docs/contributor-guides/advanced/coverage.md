<!---
name: Coverage
route: /docs/cpp/advanced/coverage
parent: Documentation
menu: Advanced
description: Describe how UTBotCpp obtain coverage information
--->

# Coverage

When user wants to get information about coverage achieved by UTBot tests, UTBot recompiles test files and user code
with coverage instrumentation, using gcov or llvm-cov, depending on the chosen compiler for running tests. This is done
by changing environment variables that affect UTBot Makefile. UTBot collects line coverage for user project files while
running tests and displays it in VSCode plugin. Major advantage of UTBot is that it shows real project coverage
collected while running native binaries. If one wants to use UTBot in its command line mode, it is also able to report
coverage and place summary files in a specified directory.