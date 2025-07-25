//
// Created by Phil Nash on 24/07/2025.
//

#include "catch23/catch23_test.h"
#include "catch23/catch23_generators.h"
#include "catch23/catch23_local_test.h"

TEST("Generators", "[.]") {
    auto i = GENERATE(values_of<int>()); // Defaults to 100 values

    CHECK( i < 50 ); // Half of these should fail
}

TEST("Two generators", "[.]") {
    auto i = GENERATE(10, values_of<int>{ .up_to=16 });
    auto j = GENERATE(10, values_of<int>{ .up_to=32 });

    CHECK( i < j ); // about 25% of these should fail
}

TEST("string generators") {
    auto words = GENERATE(10, values_of<std::string>{} );
    auto symbols = GENERATE(10, values_of<std::string>{.charset=Charsets::symbols} );

    CHECK( words != symbols );
}

TEST("section within a generator", "[.]") {
    GENERATE(100, values_of<int>{} ); // Just repeat 100x, !TBD: add a REPEAT macro to do this?

    auto results = LOCAL_TEST() {
        auto i = GENERATE(10, values_of<int>{.from=0, .up_to=10 } );
        SECTION("for small values of i") {
            CHECK(i < 0);
        }
        if( i < 5) {
            SECTION("for large values of i") {
                CHECK(i > 0);
            }
        }
    };
    CHECK(results.size() >= 10);
    CHECK(results.size() <= 20);
}

#include "catch23/catch23_catch2.h"

// From Catch2

TEST_CASE("Random generator", "[generators][approvals]") {
    SECTION("Infer int from integral arguments") {
        auto val = GENERATE(4, values_of<int>{.from=0, .up_to=1});
        REQUIRE_STATIC(std::is_same_v<decltype(val), int>);
        REQUIRE(0 <= val);
        REQUIRE(val <= 1);
    }
    SECTION("Infer double from double arguments") {
        auto val = GENERATE(4, values_of<double>{.from=0., .up_to=1.});
        REQUIRE_STATIC(std::is_same_v<decltype(val), double>);
        REQUIRE(0. <= val);
        REQUIRE(val < 1);
    }
}

// !TBD: Not sure I agree this should work in the way #1913 suggests.
// (https://github.com/catchorg/Catch2/issues/1913)
TEST_CASE("#1913 - GENERATE inside a for loop should not keep recreating the generator", "[.][regression][generators]") {
    static int counter = 0;
    for( int i = 0; i < 3; ++i ) {
        int _ = GENERATE(from_values({1,2}));
        (void)_;
        ++counter;
    }
    // There should be at most 6 (3 * 2) counter increments
    REQUIRE(counter < 7);
}

TEST_CASE("#1913 - GENERATEs can share a line", "[regression][generators]") {
    int i = GENERATE(from_values({1, 2})); int j = GENERATE(from_values({3, 4}));
    REQUIRE(i != j);
}

namespace {
    struct throwing_generator {

        auto generate() const -> int {
            throw std::logic_error("This should, actually, happen");
        }
    };
}

TEST_CASE( "#2615 - Throwing in constructor generator fails test case but does not abort",
           "[!shouldfail][regression][generators][.]" ) {
    // this should fail the test case, but not abort the application
    auto sample = GENERATE( throwing_generator() );
    // this assertion shouldn't trigger
    REQUIRE( sample == 0 );
}