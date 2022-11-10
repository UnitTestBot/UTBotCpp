---
name: Bug report
about: Create a report to help us improve
title: ''
labels: bug
assignees: ''

---

**Description**
There are hundreds of error messages in the UTBot log. Looks like the problem is in the concrete executor.

**To Reproduce**
Steps to reproduce the behavior:
1. Open the 'X' project in IDE
2. Use plugin to generate tests
3. Run the generated test

**Expected behavior**
Tests are supposed to be generated.

**Actual behavior**
An error test is generated with information about errors in the concrete executor.

**Visual proofs (screenshots, logs)**
```cpp
void testFail_errors() {
// Couldn't generate some tests. List of errors:
}
```

**Environment**
_Substitute this text with an information that can help us to recreate the environment. For instance, specify server and client operation systems, UTBotCpp version and so on._

**Additional context**
Add any other context about the problem here.
