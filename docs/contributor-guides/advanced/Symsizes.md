# Symbolic Size

## Overview

For background, refer to [Symcretes](https://github.com/UnitTestBot/UTBotCpp/wiki/Symcretes).

In symbolic execution, the objects of symbolic size is a feature that allows for dealing with dynamic memory
allocation: both size and address of an object become symcretes. To reduce memory consumption, we try to
keep memory models as small as possible, and we use the new `SparseStorage` entity to save responses from solvers for
huge allocations.

## Problem

Dynamic memory allocation is a common operation. Consider the example:

```cpp
void *allocate(int size) {
  void *storage = malloc(size);
  return storage;
}
```

If we try to run original KLEE on this code with the symbolic argument named `size`, it will approximate the size of
the allocated memory with the smallest possible power of 2. Often it is not enough, as we usually want to
know the actual size of an object.

For example, if we allocate memory and convert the type of the corresponding object, we expect the size of allocated
memory to be suitable for all the types we use it for.

In the original KLEE engine, the way of allocating memory does not imply changing the size for the memory object
during execution. Take a look at the following code snippet:

```cpp
int n = klee_int("n");
char *s = (char *)malloc(n);
if (n > 1) {
  s[1] = 20;
  if (n > 2) {
    return 0;
  }
} 
return 1;
```

In this example, the function will never `return 0`.

## How symcretes help

Each symbolic size allocation requires a pair of an address value and a size value, which are made symbolic. An idea
of making addresses symbolic appears from the fact, that they may have different values for different sizes. As we
want to maintain a model for the current memory objects, we make both sizes and addresses symcrete.
We create additional array sources for both sizes and addresses (`SizeSource` and `AddressSource` respectively).

As we are trying to model memory addresses using the system `malloc` function, the solver-suggested addresses are
usually inappropriate. To generate appropriate values, we introduce an entity called `AddressGenerator`.

## Address Generator

It is an interface having one method: `allocate(size_t size)`. All the classes implementing `AddressGenerator`
should provide appropriate (e.g. non-overlapping) addresses each time the `allocate()` method is called. Thus, an
address generator can be considered as a solver for memory addresses.

The `AddressGenerator` interface is implemented as `lib/Core/AddressManager.h`. It has the `allocate()` method and introduces
the `allocateMemoryObject()` method. For optimization purpose, the `allocate()` method allocates memory objects with 
sizes equal to powers of 2, if required. The `allocateMemoryObject()` method returns the pre-allocated 
`MemoryObject` of required size with respect to the symcrete address array.

If we want to change the size of the memory object, we allocate a new one of required size, but we must copy all the 
other properties of the original object. We can use `AddressGenerator` if we already have a memory object with 
the required properties. _Note:_ the same memory objects of different sizes have the same `ID`.

## Combining Symcretes with Address Generator

If we want to change the size of the previously allocated memory object, we allocate additional memory. Couldn't we
always allocate the biggest memory pool? In this case, we would change addresses in execution states without
changing sizes (except the one with the changed size). This may break constraint validity in these execution states,
so it is not a good practice. For the execution state where the size is changed, we would simply return an
address to a new size allocation.

The object memory size is changed in `ConcretizingSovler::relaxSymcreteConstraints()`. When we remove the equality
constraints (symcrete constraints) from a query, we assign the received addresses to the symbolic arrays and check
if the query evaluates to `true`. If it evaluates to `false`, the relaxation is impossible.

## Reducing memory consumption

To reduce memory consumption, we introduce a minimization mechanism implemented in `SolverImpl` — an interface
for the optimizing solvers.
The `computeMinimalUnsignedValue(const Query &query)` function is used to compute the minimal unsigned value for an
expression in `query.expr`. It solves the search version of an NP-complete problem via binary search with the
decision version of the SMT.

First, it attempts to find the right border for the given value: the minimal value should be less than this border.
We should multiply the border by two until we find the value, but for huge allocations it may take many queries to
gain the solution (e.g., if we meet a size constraint `if (size > 100'000) { ... }`). To avoid this problem, we
perform a bitwise binary search to find an appropriate power of 2. As soon as we compute the right border we start
usual binary search to find the minimal value.

To optimize huge allocations, we implement `SparseStorage` — an array
with the default value (similar to `std::unordered_map`, but it does not allocate memory on every read memory access).

We allocate the necessary memory object within the limit specified in `--max-sym-alloc` option.
For huge allocations, we can either leave the state unexplored or return `NULL` to model the behavior of the
`malloc` function. Consider the following code:

```cpp
char *s1 = (char *)malloc(n + 1);
char *s2 = (char *)malloc(n - 1);
if (n == 0) {
  s1[0] = 10;
}
```

Leaving the state unexplored prevents us from reaching the line `s1[0] = 10;`. Returning `NULL` would be more
appropriate here. We cannot mock the calls to `malloc`, so the resulting test becomes
unreproducible. We introduced a command-line flag to explore the paths with the "out of memory" allocations 
(`--out-of-mem-allocs`). As soon as we can mock the allocating functions, this behavior should be set as default.

## Updating execution states

As soon as relaxation is performed in `ConcretizingSolver`, we receive a solution for the given state constraints
including the new one we get from `ConcretizationManager`. We need to update the memory object in the address space
only if we add a new constraint within `Executor::addConstraint()` function. To do this, we ask 
`ConcretizationManager` for new symcrete constraints and update memory objects.

## Uninitialized memory problem

KLEE does not handle uninitialized memory properly. Consider the following example:

```cpp
int n = klee_int("n");
int *s = (int *)malloc(8);
if (n > 0) {
  s[n] = 10;
  assert(s[0] == 0);
}
```

In this example, KLEE will not be able to fail the assertion as uninitialized memory will be initialized with `0` by default.
According to C standard, this behavior is unspecified: two sequential reads from an uninitialized memory cell may return
different values. This behavior have not been supported in KLEE yet. So we decided to remain the old KLEE behavior 
(filling uninitialized memory with zeros). To handle this situation, we need to guarantee that the reads from the 
memory by an index exceeding the current concrete size always evaluates to 0.

One can do this using quantifiers, which may be complex enough for the solver — this solution reduced the performance 
of our patch with objects of symbolic size compared to original KLEE when testing with 
a [SV-Comp](https://sv-comp.sosy-lab.org/) benchmark.

Alternatively, one can use the Z3 “constant arrays” feature: arrays with predefined values for each array index. To
create this kind of arrays, a new symbolic source was added — `ConstantWithSymbolicSizeSource`.

## Evaluation

The results of a SV-Comp benchmarking study is available on shared 
[external storage](https://drive.google.com/drive/folders/1csHrTXYSk9snyDMQfKQbsyEwlQkfKZqq?usp=sharing).