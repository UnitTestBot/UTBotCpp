# Symcretes and Concretizing Solver

## Overview

Symcretes are *sym*bolic variables associated with con*crete* values. This functionality is implemented as a patch
for the KLEE symbolic execution engine. When used as part of a concolic memory model in KLEE, symcretes allow to
reduce the computational complexity for most of the queries to an SMT solver.

They also help to maintain complex features, such as lazily initialized objects or objects of symbolic
size. Symcretes make it possible to hold an appropriate model for each state along the execution path.

Formally, this feature is a modified _Observer_ pattern for `ExecutionState`. It subscribes
to a solver and decorates the queries. It handles the received responses to modify internal objects.

## Benefit

During symbolic execution, we may receive multiple symbolic values, e.g., reading from symbolic arrays, or complex
expressions like _xor_ for sums and products. Solvers take time to deal with such kind of constraints. Suggesting a
solution for a part of them may optimize computation of these queries.

The feature allows to maintain a concrete model for the objects marked as symcretes during symbolic execution, and
dynamically modify them.

## Array sources

During symbolic execution, symbolic arrays may have different sources, so they get a new characteristic —
`SymbolicSource`, i.e., an array source. Array source indicates how an array has been created. For example, arrays
containing symbolic addresses differ from arrays holding symbolic object content. To create an array source, one can call
`SourceBuilder`. Usually, array sources are stateless — in this case, `SourceBuilder` just returns the
corresponding static field values (`ConstantSource` or `MakeSymbolicSource`). Sometimes array sources do have
parameters (like `ConstantWithSymbolicSize` having the `defaultValue` field), so the new source instances are created.

## Symcrete

Symcrete is an association between a symbolic array, and an exact value set for it. These associations are
explicitly stored as `Assignment` in `ConstraintsSet`. Public interface for using `ConstraintSet` is:

* `Constraints::updateConcretization`
* `Constraints::getConcretization`

To use symcretes effectively, one should engage `ConcretizationManager` and `ConcretizingSolver`.

## Concretizing solver

Concretizing solver is the core solver for symcretes.

For every query that should be sent to an SMT solver, the concretizing solver adds symcrete constraints —
equality constraints between symbolic expressions and concrete values.

This functionality was added to all optimizing solvers inside the KLEE engine. Previously, all of them could only 
ask if a query was true or compute initial values. From now, each solver returns either `ValidResponse` or `InvalidResponse` 
(we are working with a theorem prover, not just a SAT solver).

* `ValidResponse` is a response containing `validityCore` — formulas that make a theorem valid.
* `InvalidResponse` is a response containing counterexample values for a query — values that make the theorem
  invalid.

We usually expect to get `InvalidResponse` from the concretizing solver: concretizing symcretes with current
`ConstraintSet` may accidentally prove the query (as we restricted values with symcrete constraints). To overcome
that, we should remove all the symcrete constraints and re-send a non-concretized query to the SMT solver. Usually,
we do not remove all of them. To identify those affecting validity, we look into `validityCore` and search for all 
the constraints related to symcretes — we get symcrete-related arrays from
`validityCore` by their sources. As soon as we remove these symcrete-related arrays, we send the partly-concretized 
query to the SMT solver. We can repeat this operation as validity cores do not always hold all the constraints 
causing “valid” or “invalid” verdicts (it depends on how an underlying SMT solver is implemented).

This constraint relaxation is performed in the `ConcretizationManager::relaxSymcreteConstraints` function.
Here, the additional processing is performed to do concretization properly (one of the use cases for symcretes is the
[Symbolic size](https://github.com/UnitTestBot/UTBotCpp/wiki/Symsizes) feature).

## Concretization manager

`ConcretizationManager` stores concrete values related to the symcretes received from `ConcretizingSolver` (that is
`Assignment`).

_Note:_ for each `ExecutionState` there must be symcrete constraints that do not contradict to the initial set of
constraints. In other words, we must **always** maintain the model for symcretes.

## Other applications

Symcretes make our symbolic size implementation different from similar approaches: in our patch, it is possible to 
add a value that should be interpreted as symbolic but cannot receive a model from an SMT solver.

Symcretes may be useful for fuzzing the external calls, which are challenging for symbolic execution. When we do not 
know exactly what the function does, we can find the possible argument values and make a direct call to the function.
These values should comply with the function properties. In other words, we concretize symbolic values if we meet an 
external function call.

To improve the behavior for external calls, we can use fuzzing. Each call to a function being analysed produces a 
model for both arguments and a return value. A fuzzer can be regarded as a solver with a different algorithm inside. 
We cannot treat arguments and return values as pure symbolic ones: they only can
get values from the fuzzer. And this is how we can use symcretes: to store values that cannot be purely symbolic 
but behave similarly. For the objects of symbolic sizes, we make both size and address symcrete.