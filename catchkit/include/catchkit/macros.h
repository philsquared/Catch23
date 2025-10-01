//
// Created by Phil Nash on 1/10/2025.
//

#ifndef CATCHKIT_MACROS_H
#define CATCHKIT_MACROS_H

// This header contains ONLY macros and inclusions of other macros.
// We keep these separate so they can be included when using CatchKit or Catch23 as modules.

#include "internal_warnings.h"

#define CATCHKIT_INTERNAL_ASSERT(macro_name, checker, ...) \
    if( checker.should_decompose ) \
        checker( CatchKit::AssertionContext(macro_name, #__VA_ARGS__) ) \
            .handle_unexpected_exceptions([&](CatchKit::Detail::Asserter& asserter){ \
                CATCHKIT_WARNINGS_SUPPRESS_START \
                CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON \
                asserter << __VA_ARGS__; \
                CATCHKIT_WARNINGS_SUPPRESS_END \
            }); \
    else checker(CatchKit::AssertionContext(macro_name, #__VA_ARGS__)).simple_assert(__VA_ARGS__)


#define CATCHKIT_INTERNAL_ASSERT_THAT(macro_name, checker, arg, match_expr) \
    do { using namespace CatchKit::Matchers; \
        checker(CatchKit::AssertionContext(macro_name, #arg ", " #match_expr)).that( [&]{ return arg; }, match_expr ); \
    } while( false )


#define CHECK(...) CATCHKIT_INTERNAL_ASSERT( "CHECK", check, __VA_ARGS__ )
#define REQUIRE(...) CATCHKIT_INTERNAL_ASSERT( "REQUIRE", require, __VA_ARGS__ )

#define CHECK_THAT( arg, matcher ) CATCHKIT_INTERNAL_ASSERT_THAT( "CHECK_THAT", check, arg, matcher )
#define REQUIRE_THAT( arg, matcher ) CATCHKIT_INTERNAL_ASSERT_THAT( "REQUIRE_THAT", require, arg, matcher )

#define REQUIRE_STATIC(...) static_assert(__VA_ARGS__)

// !TBD: These should have a dedicated internal macro (in Catch2 it was INTERNAL_CATCH_MSG)
#define PASS(...) CATCHKIT_INTERNAL_ASSERT( "PASS", check, true __VA_OPT__(,) __VA_ARGS__ )
#define FAIL(...) CATCHKIT_INTERNAL_ASSERT( "FAIL", require, false __VA_OPT__(,) __VA_ARGS__ )

#endif // CATCHKIT_MACROS_H
