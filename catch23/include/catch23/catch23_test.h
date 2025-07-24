//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCH23_TEST_H
#define CATCH23_TEST_H

#include "catch23/catch23_sections.h"
#include "catch23_internal_test.h"

#include "catchkit/catchkit_checker.h"
#include "catchkit/catchkit_internal_warnings.h"
#include "catchkit/catchkit_unique_name.h"

#define CATCH23_TEST_INTERNAL(fname, ...) \
    static void fname(CatchKit::Checker&, CatchKit::Checker&); \
    namespace{ CatchKit::Detail::AutoReg CATCHKIT_INTERNAL_UNIQUE_NAME( autoRegistrar ) ( { &fname, { std::source_location::current(), __VA_ARGS__ } } ); } /* NOLINT */ \
    CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER \
    static void fname(CatchKit::Checker& check, CatchKit::Checker& require ) \

#define TEST(...) CATCH23_TEST_INTERNAL(CATCHKIT_INTERNAL_UNIQUE_NAME(catch23_test), __VA_ARGS__)

#endif // CATCH23_TEST_H
