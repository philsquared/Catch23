//
// Created by Phil Nash on 1/10/2025.
//

#ifndef CATCHKIT_MACROS_H
#define CATCHKIT_MACROS_H

// This header contains ONLY macros and inclusions of other macros.
// We keep these separate so they can be included when using CatchKit or Catch23 as modules.

#include "internal_warnings.h"

#define CATCHKIT_INTERNAL_ASSERT(macro_name, checker, ...) \
    checker( CatchKit::AssertionContext(macro_name, #__VA_ARGS__) ) \
        .handle_unexpected_exceptions([&](CatchKit::Detail::Asserter& asserter){ \
            if( checker.should_decompose ) { \
                CATCHKIT_WARNINGS_SUPPRESS_START \
                CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON \
                asserter.accept_expr( asserter <=> __VA_ARGS__ ); \
                CATCHKIT_WARNINGS_SUPPRESS_END \
            } else { \
                asserter.simple_assert(__VA_ARGS__); \
            } \
        })


#define CATCHKIT_INTERNAL_ASSERT_THAT(macro_name, checker, arg, match_expr) \
    checker( CatchKit::AssertionContext(macro_name, #arg ", " #match_expr) ) \
        .handle_unexpected_exceptions([&](CatchKit::Detail::Asserter& asserter){ \
            using namespace CatchKit::Matchers; \
            asserter.that( [&]{ return arg; }, match_expr ); \
        })


#define CHECK(...) CATCHKIT_INTERNAL_ASSERT( "CHECK", check, __VA_ARGS__ )
#define REQUIRE(...) CATCHKIT_INTERNAL_ASSERT( "REQUIRE", require, __VA_ARGS__ )

#define CHECK_THAT( arg, matcher ) CATCHKIT_INTERNAL_ASSERT_THAT( "CHECK_THAT", check, arg, matcher )
#define REQUIRE_THAT( arg, matcher ) CATCHKIT_INTERNAL_ASSERT_THAT( "REQUIRE_THAT", require, arg, matcher )

// Variable capture
#define CATCHKIT_INTERNAL_DECLARE_VAR_X(suffix, var) CatchKit::Detail::TypedVariableCaptureRef CATCHKIT_INTERNAL_UNIQUE_NAME(var_capture##suffix)(var, #var, check)

#define CATCHKIT_INTERNAL_DECLARE_VARS_1(a) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a)
#define CATCHKIT_INTERNAL_DECLARE_VARS_2(a, b) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a); CATCHKIT_INTERNAL_DECLARE_VAR_X(_2, b)
#define CATCHKIT_INTERNAL_DECLARE_VARS_3(a, b, c) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a); CATCHKIT_INTERNAL_DECLARE_VAR_X(_2, b); CATCHKIT_INTERNAL_DECLARE_VAR_X(_3, c)
#define CATCHKIT_INTERNAL_DECLARE_VARS_4(a, b, c, d) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a); CATCHKIT_INTERNAL_DECLARE_VAR_X(_2, b); CATCHKIT_INTERNAL_DECLARE_VAR_X(_3, c); CATCHKIT_INTERNAL_DECLARE_VAR_X(_4, d)

#define CATCHKIT_INTERNAL_DECLARE_VARS_ERROR(...) \
    static_assert(false, \
        "CAPTURE ERROR: Too many variable names provided! " \
        "This macro supports a maximum of 4 variables. " \
        "Consider breaking into multiple CAPTURE calls.")

#define CATCHKIT_INTERNAL_GET_6TH_ARG(arg1, arg2, arg3, arg4, arg5, arg6, ...) arg6
#define CATCHKIT_INTERNAL_DECLARE_VARS_CHOOSER(...) CATCHKIT_INTERNAL_GET_6TH_ARG(__VA_ARGS__, CATCHKIT_INTERNAL_DECLARE_VARS_ERROR, CATCHKIT_INTERNAL_DECLARE_VARS_4, CATCHKIT_INTERNAL_DECLARE_VARS_3, CATCHKIT_INTERNAL_DECLARE_VARS_2, CATCHKIT_INTERNAL_DECLARE_VARS_1)

#define CATCHKIT_INTERNAL_VAR(...) CATCHKIT_INTERNAL_DECLARE_VARS_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#define CAPTURE(...) CATCHKIT_INTERNAL_VAR(__VA_ARGS__)

// Static assert
#define REQUIRE_STATIC(...) static_assert(__VA_ARGS__)

// !TBD: These should have a dedicated internal macro (in Catch2 it was INTERNAL_CATCH_MSG)
#define PASS(...) CATCHKIT_INTERNAL_ASSERT( "PASS", check, true ) __VA_OPT__(<<) __VA_ARGS__
#define FAIL(...) CATCHKIT_INTERNAL_ASSERT( "FAIL", require, false ) __VA_OPT__(<<) __VA_ARGS__

#endif // CATCHKIT_MACROS_H
