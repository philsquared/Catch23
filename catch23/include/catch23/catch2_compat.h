//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_CATCH2_H
#define CATCH23_CATCH2_H

#include "catch23/test.h"
#include "catchkit/matchers.h"

namespace CatchKit::Detail {

    auto make_test_info(std::source_location location, std::string name = {}, std::string_view tag_spec = {} ) -> TestInfo;

} // namespace CatchKit::Detail


// TEST_CASE macro is now in macros.h

#define CHECK_FALSE(...) CATCHKIT_INTERNAL_ASSERT( "CHECK_FALSE", check, CatchKit::Detail::InvertResult::Yes, __VA_ARGS__ )
#define REQUIRE_FALSE(...) CATCHKIT_INTERNAL_ASSERT( "REQUIRE_FALSE", require, CatchKit::Detail::InvertResult::Yes, __VA_ARGS__ )

#ifdef FAIL
#undef FAIL
#endif
#define SUCCEED(...) CATCHKIT_INTERNAL_BOOL_ASSERT( "SUCCEED", check, true ) __VA_OPT__(<<) __VA_ARGS__
#define FAIL(...) CATCHKIT_INTERNAL_BOOL_ASSERT( "FAIL", require, false ) __VA_OPT__(<<) __VA_ARGS__

#endif // CATCH23_CATCH2_H
