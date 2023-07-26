# Welcome to UnitTestBot C/C++ Wiki!

Generate ready-to-use test cases for your C/C++ code and feel free to contribute.

## What is UnitTestBot C/C++

UnitTestBot C/C++ is a user-friendly wrapper over the enhanced [KLEE](http://klee.github.io/) symbolic execution engine.

The KLEE engine is built on top of the LLVM compiler infrastructure and can automatically generate inputs for C code
unit testing. While it is a well-developed and maintained tool, it requires environment preparation, preprocessing,
and afterwork.

UnitTestBot C/C++ automates all these stages and provides users with a step-by-step wizard UI. It generates
ready-to-use C++ test cases in [Google Test](http://google.github.io/googletest/) format — with inputs, method
bodies, and auto-generated stubs.

## Client-server architecture

The KLEE engine requires using a specific toolchain: exact LLVM and GCC versions,
and a particular solver. The other version of the necessary utility may be installed on a user
machine — it may cause a version conflict: one cannot use different LLVM or compiler versions in a one-machine configuration.

We implemented UnitTestBot C/C++ based on a client-server model: it is a separate server where UnitTestBot C/C++
deploys the necessary _toolchain_ and performs the _resource-consuming operations_, and the client is an IDE plugin.

## Building your project

If you cannot build your own project, you will not be able to generate tests using UnitTestBot C/C++.

## Supported language features

Though KLEE is a mature project, it still supports not all the C language constructs. For UnitTestBot C/C++, we
implemented several KLEE patches to support _floating point_ types, _input/output_ operations, and _inline
assemblies_.

UnitTestBot C/C++ still has limited support for C++ features. Check supported [C](c-syntax) and [C++](cpp-syntax) syntax.