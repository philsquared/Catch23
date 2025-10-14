# Design Questions

This is somewhere to track some outstanding design questions that I'd ideally like to resolve before first release.

## Support for macro-less tests?
This support was removed as it had minimal benefit and a lot of knock-on effects.
We will wait until C++26 to be able to offer these as reflection should make it easier.

## Attaching messages to assertions

In Catch2 you had to use `INFO` before the assertion.
Many other frameworks support a syntax where you can attach messages more directly, and there is something to be said for this.
I considered (and originally implemented) a comma syntax that mirrors how you supply a message to static_assert.

e.g.:

```c++
CHECK( a != b, "One of these things is not like the other");
```

However this has the problem that, because it was within the macro, it got caught in the stringification.
So I have moved it outside the macro, "streamed in" with the << operator (similar to some other frameworks);

```c++
CHECK( a != b ) << "One of these things is not like the other";
```

I think this the best trade-off, but leaving it here for now.

## Matcher detection

There are concepts to detect Matchers, but these currently fail with overloads and constrained templates.
How can we make this handle more cases without putting more onus on Matcher authors to add extra annotations?

## Matcher API

Can we simplify how matchers are written? Current need one (or more) of:
    match(arg)
    match(arg, matcher)
    lazy_match(lambda)
    lazy_match(lambda, matcher)

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
