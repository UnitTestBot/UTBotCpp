# Types internal representation and fetching

The fetching process and the internal representation of the user's types looks like this:

![The process of fetching a struct](https://user-images.githubusercontent.com/46475907/188881079-7a00b5f1-eb7e-460b-9803-765d10004152.png)

1. Fetcher (top-right corner) uses [LibTooling](https://clang.llvm.org/docs/LibTooling.html) to obtain information about types. This library uses [callbacks](https://clang.llvm.org/doxygen/classclang_1_1ast__matchers_1_1MatchFinder_1_1MatchCallback.html) to call them when suitable condition.
2. Different callbacks (for structs and enums, for functions, for global variables, etc.) use `MatchResult` and saves information about types: name, size, and so on.
3. This information is used on further stages of tests generation. For example, on the stage of converting KLEE files to GTests.

## Fetching a struct

Let's consider fetching a struct in more details. Callback for structs and enums calls [TypesResolver](../blob/main/server/src/types/TypesResolver.h) which obtains from [Clang AST](https://clang.llvm.org/docs/IntroductionToTheClangAST.html) information about its size and recursively gets information about the fields of the struct. After all needed data about the fields is obtained, `StructInfo` is saved into the 'database' (actually, into the map `ID` --> `StructInfo`) that stores records about all structs and unions.

## Bit fields

A [bit field](https://en.cppreference.com/w/c/language/bit_field) is a way to explicitly specify a width (a size) of a member of a class. They are widely used in programming embedded systems and developing device drivers — such projects, where disk and memory space are very limited.

To add support for bit fields in UTBot, the project structure had to be redesigned a bit. 

Bit fields need to be stored with size and offset in bits, thus these properties are stored in bits for all other objects, too. 

The [stage](../wiki/utbot-inside#tests-generation) of parsing output of KLEE to final tests code assumes that objects' info is in bits as well. For example, let's consider [reading bytes as values](../blob/d1ead3c5a00b05380b59fc056f63ed024cdf9926/server/src/Tests.h#L818): the array of bytes of an object, the offset in bits and the size of the object in bits are passed to the function, and it returns the value of the type of the parsed object. To implement this, a lot of bit shifts are used. The other possible implementation is using a [bitset](https://en.cppreference.com/w/cpp/utility/bitset), but it isn't a good option because of compile-time fixed size of bitsets.

### Unnamed bit fields

To support unnamed bit fields, the [fetching](../blob/d1ead3c5a00b05380b59fc056f63ed024cdf9926/server/src/types/TypesResolver.cpp#L88) process for structure-like data uses a little trick. Because there's no meaningful use for unnamed bit fields inside the UTBot, we don't resolve [field declarations](https://clang.llvm.org/doxygen/classclang_1_1FieldDecl.html) which are representing bit fields. So instead of storing information about unnamed bit fields and ignoring them in traversers, printers and visitors, we [skip unnamed bit fields](../blob/d1ead3c5a00b05380b59fc056f63ed024cdf9926/server/src/types/TypesResolver.cpp#L128) in the fetching stage.