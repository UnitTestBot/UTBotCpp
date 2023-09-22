<!---
name: Stubs inside
route: /docs/cpp/advanced/stubs-inside
parent: Documentation
menu: Advanced
description: Describe how UTBotCpp stubs work
--->

The paradigm of unit testing is to verify correctness of code units in isolation. There may be many opinions on what is
a unit: a function, a file, a class or even a library; UTBot defines a unit as the smallest project target that contains
selected code. For example, if it code from file1.c is tested (see Figure 5), then helper.bc
(rather then exe1.bc) will be selected as its unit. When tests for unit are generated, there may be an interest to test
them with respect that when functions from another unit are called, their complete testing is unwanted. It is a common
practice to use stubs for such functions — functions with same signatures, that execute simplified behaviour, which
allows testing process to focus on a selected unit.

To support this intention, UTBot offers a possibility to use symbolic stubs — stubs, which, when called, return a
symbolic value. Functions from units other than the tested one can be replaced with such stubs, and selected unit will
be tested, regarding that other units’ functions may return any value.

```c++
int process_temperature() {
    int t = read_t_sensor();
    if (t < 0) {
        printf("Negative");
        return -1;
    }
    if (t < 30) {
        printf("Comfortable");
        return 0;
    }
    printf("Too hot");
    return 1;
}
```

An example of using stubs. Consider a function above. If one wants to test various execution paths of
process_temperature()
function, their may not be interested in testing read_t_sensor()
that can address real physical device and require complex environment to setup.

Stub file description. Code example bellow presents UTBot stub file example. Its contents are not totally intuitive, because the
same stub files are used for generating tests (translate to LLVM bitcode and interpret by KLEE) and running them. When
stubs are compiled to be passed to KLEE, KLEE_MODE=1 is assigned during compilation, so calling a stub from tested unit
for the first time would create a symbolic variable, and then this variable will be returned for every function call.

```c++
//automatically generated
char read_t_sensor_symbolic;
char read_t_sensor() {
static int firstTimeCall = 1;
#ifdef
KLEE_MODE
if (firstTimeCall == 1) {
firstTimeCall = 0;
klee_make_symbolic(
&read_t_sensor_symbolic,
sizeof(read_t_sensor_symbolic),
"read_t_sensor_symbolic");
}
//can be manually added
klee_prefer_cex(read_t_sensor_symbolic == 0);
klee_assume(read_t_sensor_symbolic > -273);
#endif
//automatically generated
return read_t_sensor_symbolic;
}
```

UTBot is capable of generating only simple stubs, which do not depend on their arguments, but it affords its users a
great opportunity to modify stubs themselves, using a powerful [KLEE API](https://klee.github.io/docs/intrinsics/). This can be really helpful to add
constraints to program arguments and return value, making it look like a real function, but with the advantage of much
faster symbolic execution. Consider read_t_sensor(), which, in reality, should never return values below absolute zero.
If a function like read_t_sensor() needs to be stubbed, users can add a constraint to it, which will discard all the
unwanted paths and make stub behavior closer to real function behavior. Moreover, users can apply a soft constraint to
the read_t_sensor() result to force the testing of a path where read_t_sensor() returns 0.

For each stub, a header is generated which contains symbols from user source file, needed for function signature
successful compilation. This includes structures, classes and type definitions.

![Stubs linking behaviour for c-example project](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/stubsExample.png)

Generating tests with symbolic stubs. When UTBot generates tests, it provides an option to replace all project targets
except the tested one with stubs. Prior to tests generation, UTBot creates a stub file for every source file in the
project. When the generation request specifies to use stubs, UTBot links all project targets except one not from source
files, but from stub files. UTBot uses exactly the same commands for stubs compilation and linkage, as it would use for
source files. For example, when tests generation for helper.a with stubs is requested in c-example project, targets will
be replaced to stubs according to figure above.

Running tests with symbolic stubs. Apart from generated tests, UTBot also generates Makefiles(Section 3.9) to run them.
This Makefiles contains commands to build each test with or without stubs, and commands to run built test with/without
stubs. When a test for function is generated using stubs, the test binary is linked with stub code. This time,
KLEE_MODE=0 is passed and stub function simply returns a global variable, which is set in test body. For more
information on running UTBot tests, see "Generating and running tests" (Section 3.7).