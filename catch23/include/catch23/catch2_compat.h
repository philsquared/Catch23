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

// !TBD: these lose the expression decomposition, currently (which defeats the purpose)
#define CHECK_FALSE(...) CATCHKIT_INTERNAL_ASSERT_THAT( "CHECK_TRUE", check, __VA_ARGS__, is_false() )
#define REQUIRE_FALSE(...) CATCHKIT_INTERNAL_ASSERT_THAT( "REQUIRE_TRUE", require, __VA_ARGS__, is_false() )

#define SUCCEED(...) PASS(__VA_ARGS__)

#endif // CATCH23_CATCH2_H
