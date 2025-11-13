//
// Created by Phil Nash on 1/10/2025.
//

#ifndef CATCHKIT_CATCH23_MACROS_H
#define CATCHKIT_CATCH23_MACROS_H

// This header contains ONLY macros and inclusions of other macros. The one exception is <source_location>
// We keep these separate so they can be included when using Catch23 as a module.

#include <source_location>

#include "catchkit/internal_warnings.h"
#include "catchkit/internal_macro_utils.h"
#include "catchkit/macros.h"

// Test cases
#define CATCH23_MAKE_TEST(fname, ...) \
    CatchKit::Detail::Test( &fname, { std::source_location::current()__VA_OPT__(, CATCHKIT_VA_MACRO_HEAD(__VA_ARGS__)) } ) CATCHKIT_VA_MACRO_TAIL(__VA_ARGS__)

#define CATCH23_INTERNAL_TEST2(fname, test_decl) \
    static void fname(CatchKit::Checker&); \
    namespace{ namespace CATCHKIT_INTERNAL_UNIQUE_NAME(reg_ns) { using namespace CatchKit::Tags; CatchKit::Detail::AutoReg auto_registrar ( test_decl ); } } /* NOLINT */ \
    CATCHKIT_WARNINGS_UNSCOPED_SUPPRESS_UNUSED_PARAMETER \
    CATCHKIT_WARNINGS_UNSCOPED_SUPPRESS_SHADOW \
    static void fname(CatchKit::Checker& checker )

#define CATCH23_INTERNAL_TEST(fname, ... ) CATCH23_INTERNAL_TEST2( fname, CATCH23_MAKE_TEST(fname, __VA_ARGS__) )

#define TEST(...) CATCH23_INTERNAL_TEST( CATCHKIT_INTERNAL_UNIQUE_NAME(catch23_test_), __VA_ARGS__ )

// Sections
#define SECTION(name) if( auto section_info = try_enter_section(CatchKit::Detail::get_execution_nodes_from_result_handler(*checker.result_handler), name) )

// Meta testing
#define LOCAL_TEST(...) CatchKit::MetaTestRunner(__VA_ARGS__) << [](CatchKit::Checker& checker)
#define RUN_TEST_BY_NAME(name) CatchKit::MetaTestRunner(name).run_test_by_name( name )

// Generators
#define GENERATE(...) \
    [&checker]{ using namespace CatchKit::Generators; \
        CatchKit::Detail::GeneratorAcquirer acquirer(checker, {#__VA_ARGS__}); \
        if( !acquirer.generator_node ) acquirer.make_generator((__VA_ARGS__)); \
        return acquirer.derived_node<decltype((__VA_ARGS__))>(); \
    }()->current_value()


// Catch2 compatibility
#define CATCH23_INTERNAL_MAKE_TEST_LEGACY(fname, ...) \
    CatchKit::Detail::Test( &fname, CatchKit::Detail::make_test_info( std::source_location::current() __VA_OPT__(, __VA_ARGS__ ) ) )
#define CATCH23_INTERNAL_TEST_LEGACY(fname, ... ) CATCH23_INTERNAL_TEST2( fname, CATCH23_INTERNAL_MAKE_TEST_LEGACY(fname, __VA_ARGS__ ) )

#define TEST_CASE(...) CATCH23_INTERNAL_TEST_LEGACY(CATCHKIT_INTERNAL_UNIQUE_NAME(catch23_test), __VA_ARGS__)


#endif // CATCHKIT_CATCH23_MACROS_H
