//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCH23_TEST_H
#define CATCH23_TEST_H

#include "catch23/sections.h"
#include "internal_test.h"

#include "catchkit/checker.h"
#include "catchkit/internal_warnings.h"
#include "catchkit/unique_name.h"

#define CATCH23_VA_MACRO_HEAD(first, ...) first
#define CATCH23_VA_MACRO_TAIL(first, ...) __VA_ARGS__

#define CATCH23_MAKE_TEST(fname, ...) \
    CatchKit::Detail::Test( &fname, { std::source_location::current()__VA_OPT__(, CATCH23_VA_MACRO_HEAD(__VA_ARGS__)) } ) CATCH23_VA_MACRO_TAIL(__VA_ARGS__)
#define CATCH23_MAKE_TEST_LEGACY(fname, ...) \
    CatchKit::Detail::Test( &fname, CatchKit::Detail::make_test_info( std::source_location::current() __VA_OPT__(, __VA_ARGS__ ) ) )

#define CATCH23_TEST_INTERNAL3(fname, test_decl) \
    static void fname(CatchKit::Checker&, CatchKit::Checker&); \
    CATCHKIT_WARNINGS_SUPPRESS_START \
    namespace{ namespace CATCHKIT_INTERNAL_UNIQUE_NAME(reg_ns) { using namespace CatchKit::Tags; CatchKit::Detail::AutoReg auto_registrar ( test_decl ); } } /* NOLINT */ \
    CATCHKIT_WARNINGS_SUPPRESS_END \
    CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER \
    CATCHKIT_WARNINGS_SUPPRESS_SHADOW \
    static void fname(CatchKit::Checker& check, CatchKit::Checker& require )

#define CATCH23_TEST_INTERNAL2(fname, test_decl) static void CATCHKIT_INTERNAL_UNIQUE_NAME(test_)()

#define CATCH23_TEST_INTERNAL(fname, ... ) CATCH23_TEST_INTERNAL3( fname, CATCH23_MAKE_TEST(fname, __VA_ARGS__) )
#define CATCH23_TEST_INTERNAL_LEGACY(fname, ... ) CATCH23_TEST_INTERNAL3( fname, CATCH23_MAKE_TEST_LEGACY(fname, __VA_ARGS__ ) )

#define TEST(...) CATCH23_TEST_INTERNAL( CATCHKIT_INTERNAL_UNIQUE_NAME(catch23_test_), __VA_ARGS__ )

#endif // CATCH23_TEST_H
