//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_CATCH2_H
#define CATCH23_CATCH2_H

#include "catch23/catch23_test.h"
#include "catchkit/catchkit_matchers.h"

#define TEST_CASE(...) CATCH23_TEST_INTERNAL(CATCHKIT_INTERNAL_UNIQUE_NAME(catch23_test), __VA_ARGS__)

// !TBD: these lose the expression decomposition, currently (which defeats the purpose)
#define CHECK_FALSE(...) CATCHKIT_ASSERT_THAT_INTERNAL( "CHECK_TRUE", check, __VA_ARGS__, is_false() )
#define REQUIRE_FALSE(...) CATCHKIT_ASSERT_THAT_INTERNAL( "REQUIRE_TRUE", require, __VA_ARGS__, is_false() )

#define SUCCEED(...) PASS(__VA_ARGS__)

#endif //CATCH23_CATCH2_H
