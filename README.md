# Catch23
A new test framework, in the spirit of Catch/ Catch2, but rewritten from the ground up for C++23 and on

This is an early work-in-progress and is not intended for general use.

What's different?
The vast majority is purely internal - which not only makes maintenance far easier (and enables new possibilities) 
but should improve runtime - and may help with compile times, too (not currently measured).

But there are some differences and new capabilities, too. 
The following taster assumes some familiarity with Catch2.

## Simpler macros

We still have `CHECK` and `REQUIRE` which work much as before (with some new abilities).
We also have `CHECK_THAT` and `REQUIRE_THAT` for matchers. 
Matchers themselves are heavily updated and more of the special case macros from Catch2 are covered by Matchers, now.

So, for now, those four macros are pretty much it 
(there's also a couple of convenience macros: `CHECK_FALSE` and `REQUIRE_FALSE` - 
which use matchers under the hood, and `FAIL` and `SUCCEED` - which call onto `REQUIRE(false)` and `REQUIRE(true)`, for now).

## New Matchers system

Matchers are instrumental to how Catch23 works.
You can think of `CHECK` and `REQUIRE` as special cases of matchers (although they have their own code paths for now).

Matchers are both simpler to write and also more powerful - all thanks to modern C++ (mostly C++20)!

Initially the main difference, visually, is just the naming convention (I may offer interop wrappers).
They are now all snake_case, and mostly configurable through optional template arguments (which, can be completely omitted when the defaults are sufficient):

```c++
CHECK_THAT("CaSe sEnsItIVe", equals("case SENSITIVE"));
CHECK_THAT("CaSe InsEnsItIVe", equals<CaseInsensitive>("case INSENSITIVE"));

CHECK_THAT("once upon a time",
    contains("upon") && starts_with("once") && contains("time"));
```

Matchers can now be eager or lazy. Lazy matchers take a lambda rather than a direct value.
The lambda may return the value when called - or needn't return a value at all!
This is all handled, transparently by the matchers system, and the Matchers themselves.
This allows a `throws` matcher that takes a lambda and tests if it throws an exception when invoked.
If it _does_ then it can optionally check the exception type,
and optionally extract an exception message and pass it on to further matchers using the `with_message()` or `with_message_that()`
member functions of the throws matcher.

This ability completely replaces the `_THROWS_` family of special macros:

```c++
CHECK_THAT( throw std::domain_error("hello"), throws() );
CHECK_THAT( throw std::domain_error("hello"), throws("hello") );
CHECK_THAT( throw std::domain_error("hello"), throws().with_message("hello") );
CHECK_THAT( throw std::domain_error("hello"), throws<std::domain_error>() );
CHECK_THAT( throw std::domain_error("not std::string"), throws<std::string>() );

CHECK_THAT( throw std::domain_error("hello"),
    throws<std::domain_error>().with_message_that( starts_with<CaseInsensitive>("heL") && contains("ll") ));
```

To write your own matcher you just need either a `match` or `lazy_match` member function (which can be a template) -
as long as it returns `MatchResult` (which can be constructed from bool) and a `describe` member function which returns a string describing the matcher (much as in Catch2).

A full description of writing custom matchers is beyond the scope of this teaser - but there's not a lot more to it.
No base classes, no pointers, no heap allocations (except any you need internally).

## String conversions

Being able to convert objects to strings is an essential ingredient to any test framework.
Catch2 used some nasty template/ overload tricks to use `std::ostreamstream` as a fallback, where available,
and a set of template specialisations for common types. You could write your own specialisations for your own types.

That's all true in Catch23, except the specialisations are more robust and powerful thanks to C++20 concepts.
The fallback to `std::ostreamstream` is optional, and is accompanied by a fallback to `std::format` (also configurable).

But I'm saving the best for last. In Catch23, enums are automatically covertible to strings (and we don't even have reflection yet)!
That's with the caveat that it uses a nasty tricky involving `std::source_location` and some templated probing.
It's a cutdown version of the technique used by libraries like Magic Enum.

## Macros? Where we're going we don't need macros!

Some modern test frameworks, using C++20 facilities, have been able to avoid using macros altogether.
Catch23 allows the same - but you do forego a couple of capabilities (or need to add a lot of boilerplate):

```c++
int const two = 2;
check() << 1 == two;

using namespace CatchKit::Matchers;
check() << "hello", starts_with("hell") && ends_with("lo");
require.handle_unexpected_exceptions( [&]{ require("this_throws()") << this_throws(); } );
```

The first two examples work as-is, but any unexpected exceptions are not recoverable (the cancel the test),
and we don't get the original expression captured as a string. The final example shows how we can get them back:
`require.handle_unexpected_exceptions` gives us the exception handling, 
and the string passed to the inner `require()` call serves the role of the expression string capture.

Indeed, the macros essentially just do these two things for you, now (not exactly, as they pass an AssertionContext to `require()`/`check()`).

So the macros are still the recommended approach, but if you want to go macro free it's good to know that you can.

## Getting the message

Catch2 has macros like `INFO` and `CAPTURE` for capturing extra strings that can printed along with the results.
Some frameworks support a syntax like using `<<` after an assertion to "stream" a message in.

Catch23 allows you to tag a string onto an assertion as just an extra argument:

```c++
CHECK( 6*9 == 42, "This is not the answer you were looking for" );
```

This works with Matchers, too:

```c++
std::vector ints{ 1, 2, 3 };
CHECK_THAT( ints, equals(std::vector{1, 2, 3}),
    "It's as easy as that" );
```

## Separation of concerns

Catch23 is actually two (will possibly be three) libraries!

*CatchKit* gives you all the assertion facilities (with expression decomposition), matchers and string converters.
It also gives you a default assertion handler that acts like the `assert()` macro - it aborts on failure.
So you can use CatchKit on its own to get Catch-like assertions that you can put in your production code!

*Catch23* builds on CatchKit to give you test cases, test runners, reporters and a command line interface.
Some of this maybe further split out but that is not clear, yet.

At time of writing the test case support (with automatic registration) is there (you can use `TEST` or `TEST_CASE`).
Tags are supported, but not yet parsed (except for "[." to hide tests).
There is a minimal test runner and a basic ConsoleReporter. There is no command line interface.

## What's next

In progress right now is support for `SECTION`s, as well as generators (they both run on something called "Execution Nodes").
The features are there and working, but need more testing and fleshing out.

Currently, the generators interface is looking like this (same idea as Catch2, but more streamlined implementation):

```c++
auto value = GENERATE( 100, values_of<int>(.up_to=42) );
```

The `100,` syntax is under consideration. Without it the default is 100 values.
Writing your own generator is very easy. E.g. here's how a string generator is implemented:

```c++
template<>
struct values_of<std::string> {
    size_t min_len = 0;
    size_t max_len = 65;
    
    auto generate() -> std::string;
    // Impl does the actual generating of a random string, given that data    
};


// ...

auto str = GENERATE( values_of<std::string>(.max_len=255) );
```

The rest is, as they say, in the details...