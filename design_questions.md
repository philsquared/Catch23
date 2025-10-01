# Design Questions

This is somewhere to track some outstanding design questions that I'd ideally like to resolve before first release.

## Support for macro-less tests?
You can currently do, e.g. `check() << a == b` (any expressions after the `<<`), or `check().that( /* matcher */ )`.
But this comes with a number of trade-offs:
1. It doesn't capture the expression as you wrote it (for printing as  a string).
2. It doesn't catch unexpected exceptions (these are caught at the test case level, so you lose specific file/ line).
3. The syntax, especially the use of `<<`, becomes part of the API, so needs to be maintained.
4. Parts of the syntax generates warnings and SA violations. Within the macro these are suppressed.
5. It complicated the expression decomposition design (see below)

Macro-less test cases are also possible, but haven't been implemented yet.
The question is, is it worth it? Maybe we wait until C++26 to be able to offer these? 
Reflection should make it easier.

## Performing assertion evaluation in destructor

Within a `CHECK` or `REQUIRE` an expression is decomposed into a XXXExprRef type (UnaryExprRef, BinaryExprRef).
Because we don't know what the final type will be until the end of the expression (it starts out as a UnaryExprRef,
but if a comparison operator is used then it becomes a BinaryExprRef) the expr objects themselves don't know if they
hold the final expression until their destructors are called.
So, currently, the ...Expr destructors call back into Asserter (to which a pointer is being held) to evaluate the expression.

We could pass the whole ...Expr object to a function that will perform the evaluation on the final object without requiring the
expr objects themselves to trigger this. The main reason for not doing that is to keep the macro-less syntax simpler.
Because the `<<` operator only binds to the first part of the expression we'd need something else to wrap the whole thing
and that would become something the user would have to write if not using macros.

## Attaching messages to assertions

In Catch2 you had to use `INFO` before the assertion.
Many other frameworks support a syntax where you can attach messages more directly, and there is something to be said for this.
For now I have added support for adding a message after a , operator overload.

e.g.:

```c++
CHECK( a != b, "One of these things is not like the other");
```

This "works" but has some downsides:

1. It may not be immediately obvious what it's doing
2. It gets caught in the `#__VA_ARGS__` conversion to string. There is an attempt to filter it out - but that breaks if you, e.g. use std::format.
3. It involves an overloaded comma operator!

(2) Is probably the biggest concern, and would apply to any solution that was "in-macro" (unless a way of avoiding it can be found).
Some other frameworks allow you to tag the message outside the macro, usually using the `<<` operator.
I'm not sure how that could be done in our case since we're in a do...while. 
It's not obvious how we could change that without other trade-offs

## Matcher detection

There are concepts to detect Matchers, but these currently fail with overloads and constrained templates.
How can we make this handle more cases without putting more onus on Matcher authors to add extra annotations?

## Matcher API

Can we simplify how matchers are written? Current need one (or more) of:
    match(arg)
    lazy_match(lambda)
    bound_match(arg, matcher)
    lazy_bound_match(lambda, matcher)

We should probably at least be able to just have match and detect (a) second arg and (b) whether first arg is lambda.


## std::formatter for Stringified types

There is a specialisation of std::formatter for types that are specialised for Stringify.
This allows us to use std::format's support for formatting ranges, while recursing back to Stringify for custom types.
It does mean that if you have you own formatter you can't write a different Stringify for it, but that's probably a narrow case.
Any other consequences?

## check and require objects

Instances of the Checker class called `check` and `require` are passed into the test function, 
but are also available as (shadowed) global variables, which is used when outside the test function.
This lets us use a more optimised path when in the test function. But
1. Is this a good idea? and
2. Do we need two objects?
One of the reasons for having two objects is to support the macroless syntax.

## Mocking

The interface is still in a state of flux

## Generators

Should generators be written as coroutines (probably using `std::generate`)?
Should we have special support for non-random generators? (currently they get a random_number generator passed).

## Shrinking

This is still WIP. Is the API working (currently a `std::generator` coroutine, with caching)?
Anything else that can be done to simplify?

## Compile times

Not much has been specifically done to control these, yet, so they are probably terrible!
Need to set up some CT benchmarking and do some experiments.
One of the biggest questions is how much of an impact the use of lambdas makes these days.
