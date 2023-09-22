# Lazy Initialization

## Overview

Lazy initialization is a mechanism that allows symbolic execution to effectively verify code dealing with pointers,
complex data structures, and dynamic memory allocation. This technique is commonly used in many
state-of-the-art engines. We provide an advanced implementation of this mechanism as a patch for the
KLEE symbolic execution engine.

When KLEE meets pointers and makes them symbolic for further processing, it is afterwards
not obvious for the engine how to dereference a symbolic pointer. The engine iteratively scans the address space
trying to dereference the pointer to existing objects until it finds all the reasonable variants or gets an error of
having more possible (but uninitialized) objects the pointer could refer to. To explore the additional behaviors of a
program that cannot be straightforwardly deduced from the current
context, the lazy initialization feature creates an additional object in an address
space of a program under analysis.

## Example

Consider the following function:

```cpp
int *ptr;
klee_make_symbolic(&ptr, sizeof(ptr), "ptr");
foo(ptr);
// ...
void foo(int *x) {
  *x = 10;
  assert(*x);
}
```

If we try to analyze this function with original KLEE, we run this function with the symbolic argument `*x`. The engine
either fails on the line `*x = 10;` with `memory error: out of bounds`, or resolves to an existing global object (e.
g., the `errno` variable). This might be not a behavior the user expects from such kind of analyzers, as this function
is run only with arguments referencing to existing objects (on a heap or a stack), and checks invariants inside the
current address space.

To overcome this obstacle, we introduce a _Lazy initialization_ mechanism, which forces KLEE to create a new object
when a symbolic pointer can be resolved not only to existing objects but somewhere else. Get back to the example
above: with the new feature, on the line `*x = 10;` we allocate one more object of `int` type while dereferencing
the pointer, and use it in future without corrupting global objects.

Formally, an engine without a lazy initialization feature can only under-approximate program behavior, while being
enhanced with this new feature it can perform a lot more precise analysis.

## Fully symbolic memory model

We introduced the first implementation of this feature in our earlier patches for KLEE.
Speaking in more detail, lazy initialization can be applied during a memory operation (`load` or `store`) performed
in a `klee::Executor::executeMemoryOperation` function. Imagine the code under test manipulating a pointer. KLEE
makes this pointer a symbolic one — a symbolic representation of an address the given pointer points at (i.e.,
_pointee_). Upon receiving a symbolic pointer, which cannot be resolved to exactly one object, the 
`klee::Executor::executeMemoryOperation` function searches through all the
objects in the address space and checks if a pointer can dereference to one of them. Then the
`executeMemoryOperation` function forks the initial state — it is the state relevant to the moment of the
function call. If the pointer still can be resolved to additional memory
locations, KLEE initializes one more memory object with the corresponding symbolic address (`ref<Expr>` in the KLEE
code) and with the fixed size related to the pointee type.
In our example, KLEE will initialize just a new `int` object of 4 bytes. All the subsequent requests for the
address of this memory object will result in this symbolic address.

_Note:_ this approach may affect performance, as we may add more formulas to queries we send to an SMT solver.

## Symcretes

To increase KLEE engine performance while executing on pointers, we introduce an expression type named
_symcretes_. During initialization, we associate a symbolic value with a concrete value (we get the concrete value 
using `malloc`).  This association is a _symcrete_. For more information, 
refer to [Symcretes](https://github.com/UnitTestBot/UTBotCpp/wiki/Symcretes).

When all the symbolic addresses become associated with the concrete values, the queries to a solver become
simpler, and symbolic execution gets speed-up. We can consider such optimization to be the KLEE behavior
approximation. _Note:_ if an expression containing such _symcrete associations_ causes UNSAT in a solver, we do not
recompute addresses. Such UNSAT verdicts appear only if the pointers are somehow compared in a user code, leading to
unspecified behavior.

## Lazily initialized objects of symbolic size

With the symbolic size feature, we can extend the behavior of lazily initialized objects. Consider the following
example:

```cpp

int *ptr;
klee_make_symbolic(&ptr, sizeof(ptr), "ptr");
foo(ptr);

int foo(int *x) {
  x[1] = 10;
  x[2] = 2 * x[1];
  return x[2];
}
```

According to the initial implementation of the lazy initialization feature, we would allocate a memory object of 4 
bytes.
In this example, it leads to `memory error: out of bound` at the line `x[1] = 10;`. This behavior is unexpected for
this function as it seems to get arrays as arguments.

The symbolic size feature allows us to specify the size and to make it a symbolic value equal to the
size of a pointee object or greater than it.