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

#define CATCH23_TEST_INTERNAL(fname, ...) \
    static void fname(CatchKit::Checker&, CatchKit::Checker&); \
    namespace{ CatchKit::Detail::AutoReg CATCHKIT_INTERNAL_UNIQUE_NAME( autoRegistrar ) ( { &fname, { std::source_location::current(), __VA_ARGS__ } } ); } /* NOLINT */ \
    CATCHKIT_WARNINGS_SUPPRESS_UNUSED_PARAMETER \
    CATCHKIT_WARNINGS_SUPPRESS_SHADOW \
    static void fname(CatchKit::Checker& check, CatchKit::Checker& require ) \

#define TEST(...) CATCH23_TEST_INTERNAL(CATCHKIT_INTERNAL_UNIQUE_NAME(catch23_test_), __VA_ARGS__)

#endif // CATCH23_TEST_H
