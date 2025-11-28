# MVP todo

What things must be implemented (and to what degree) to constitute an MVP?

## CLI

A command line interface is definitely necessary
What things are needed straight away or soon?

* Ability to run tests by name
* Ability to run tests by tag
* Ability to specify reporter
* Ability to specify where output should be written

## Reporters

We need different reporters and a way to specify them

* Console reporter (implemented, but needs stable API)
* JUnit reporter
* XML reporter (Catch2 compatible)
* CSV reporter?
* JSON reporter?

Do we need to lock down the interface for specifying reporters in terms of format, target (e.g filename) and disposition (show passing tests etc?).

## Redirect stdout

At least std::cout, but if we can redirect stdout in general that's even better

## Custom exception translation

## Signal handling

## CI

Need to build and run tests on GitHub

* Needs Windows build
* external approval tests?
* Runtime benchmarks?
* Compile time benchmarks?

## Generators

* Flesh out at least the non-shrinking part
* Shrinkers for ints and strings?

## Matchers

* More container matchers
  * starts_with/ ends_with for containers
  * composite element matchers (e.g. "all elements must match")

## Break into debugger

## Skip tests

# Templated tests?