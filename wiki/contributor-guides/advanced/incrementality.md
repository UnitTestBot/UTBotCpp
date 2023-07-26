<!---
name: Incrementality
route: /docs/cpp/advanced/incrementality
parent: Documentation
menu: Advanced
description: Incrementality
--->

# Incrementality

Users can create multiple requests of tests generation; UTBot creates many artifacts for each of them, namely:

* UTBot KLEE files;
* UTBot stub files and headers;
* Object files and project targets for user project files, KLEE files, stub files built both in LLVM bitcode and with a
  classic C/C++compiler/linker;
* Test source files and headers;
* Test executables.
* UTBot Makefiles

Notice that many of those artifacts are reusable between test generation requests: if a test generation happens for two
files in the same project target, then all stub sources and artifacts from them, all but one project targets artifacts
built in LLVM bitcode and as native files, are already prepared by UTBot and do not need to be recreated. To control
artifacts state and reuse them, UTBot generates UTBot Makefiles for every test generation and run step. Generated
Makefiles are used for multiple purposes that are determined by environment variables, namely: compile sources, link
them into LLVM IR for KLEE, compile tests and link them into real library or executable for run, turn stubs off/on, turn
coverage off/on. When a request is made, corresponding rule from a Makefile is called. Hence, if rule dependencies are
up to date, they are reused — Makefile watches artifacts creation timestamps and is able to determine if a Makefile
target is up to date.

