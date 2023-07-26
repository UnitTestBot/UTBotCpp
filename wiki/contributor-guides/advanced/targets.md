<!---
name: Targets
route: /docs/cpp/advanced/targets
parent: Documentation
menu: Advanced
description: In some cases the behaviour of method depends on target which it is related to. 
--->

# Targets

The **target** is either an executable or root library, which is not *related* to any executable. Depending on the
structure of the project, the same file may be related to different targets and be linked with different function
implementations. For instance, stubs may or may not be used to link with tests (see [stubs](stubs)). For ease of
understanding, UnitTestBot chooses target with stubs in one case and without them in another one, while the code of
tests remains the same.

## Start

Let us consider `coreutils` project. When you open the project and go to UTBot Explorer, UTBot Targets view appears at
the top. By default, the target with the largest number of files within is chosen. It is `bench-sha224` in that case.
You may change that to your preferred target. For example, pick `cat` if you primarily develop `cat` tool.

![targetsExplorerViewImg](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/targetsExplorerView.png)

It is all right to have many targets in project. However, the default choice may not fit you. Anyway, you may always
just run test generation and rely on UnitTestBot's hints.

## File is presented in compile/link commands for chosen target, but not included in target's artifact...

Let's see what happens when you run test generation for function `parse_datetime` in `lib\parse-datetime.c` file with
default target.

![targetsFileNotPresentedInArtifactImg](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/targetsFileNotPresentedInArtifact.png)

It happens very rarely, but it does happen for a reason. Even though current file is presented in compile/link commands
for chosen target, it is not included in target's artifact. More specifically, the library `libcoreutils.a` with
function `parse_datetime` is thrown out by linker. So, it doesn't make sense to test particular function as part of
target `bench-sha224`.

Let us see what happens next if you click `Choose target` and choose `date` target instead. Great! Tests were generated
successfully.

![targetsTestsForDateGeneratedImg](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/targetsTestsForDateGenerated.png)

## File is not presented in compile/link commands for chosen target...

Let's see what happens when you run test generation for function `usage` in `src\cat.c` file with default target.

![targetsFileNotPresentedInCommandsImg](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/targetsFileNotPresentedInCommands.png)

It happens more often, but it also does happen for a reason. Function could not be a part of target `bench-sha224` in
any case, because it is not used by compile and link commands for chosen target.

Let us see what happens next if you click `Choose target` and choose `cat` target instead. Great! Tests were generated
successfully.

![targetsTestsForCatGeneratedImg](https://github.com/UnitTestBot/unittestbot.github.io/raw/source/resources/images/targetsTestsForCatGenerated.png)
