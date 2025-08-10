//
// Created by Phil Nash on 24/07/2025.
//

#include "catch23/test.h"
#include "catch23/generators.h"
#include "catch23/local_test.h"

TEST("Generators", "[.]") {
    auto i = GENERATE(values_of<int>()); // Defaults to 100 values

    CHECK( i < 50 ); // Half of these should fail
}

TEST("Two generators", "[/.]") {
    auto i = GENERATE(10, values_of<int>{ .up_to=16 });
    auto j = GENERATE(10, values_of<int>{ .up_to=32 });

    CAPTURE( i, j );
    if( i > 4 && j > 4 )
        CHECK( i < j ); // about 25% of these should fail
    else
        PASS();
}

TEST("string generators") {
    auto words = GENERATE(10, values_of<std::string>{} );
    auto symbols = GENERATE(10, values_of<std::string>{.charset=Charsets::symbols} );

    CHECK( words != symbols );
}

// #include <print>
//
// TEST("sections within a generator", "[/.]") {
//     auto i = GENERATE(inclusive_range_of<int>{.from=0, .to=10 } );
//     SECTION("a") {
//         std::println("a: {}, {}", section_info.node.get_current_index(), i);
//     }
//     SECTION("b") {
//         std::println("b: {}, {}", section_info.node.get_current_index(), i);
//     }
//     SECTION("c") {
//         std::println("c: {}, {}", section_info.node.get_current_index(), i);
//     }
// }


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

TEST("vector generators") {
    auto v = GENERATE(10,
        values_of<std::vector<int>>{
            .value_generator={.up_to=64}
        } );

    PASS();
}

#include "catch23/catch2_compat.h"

// From Phil's Accelerated TDD workshop
std::string left_pad(std::string const& input_string, int min_len) {
    if(auto pad_len = min_len-static_cast<int>(input_string.length()); pad_len > 0)
        return std::string(pad_len, ' ') + input_string;
    return input_string;
}

TEST_CASE("left pad properties", "[/.]") {
    // check.result_handler.report_on = CatchKit::ReportOn::AllResults;

    auto min_len = GENERATE(10, values_of<size_t>{0, 256});
    auto input_string = GENERATE(10, values_of<std::string>{.min_len=0, .max_len=256});
    auto output_string = left_pad(input_string, min_len);

    // CAPTURE(input_string);
    // CAPTURE(output_string);
    // CAPTURE(min_len);

    REQUIRE(output_string.length() >= min_len);
    REQUIRE(output_string.length() >= input_string.length());
    REQUIRE(output_string.find(input_string) != std::string::npos);
    REQUIRE(output_string.rfind(input_string) == output_string.length()-input_string.length());

    if( input_string.length() == min_len ) {
        SECTION("if input string length is same as min_len then input string is returned") {
            REQUIRE(output_string == input_string);
        }
    }

    if( input_string.length() > min_len ) {
        SECTION("if input string length is greater than min len then input string is returned") {
            REQUIRE(output_string == input_string);
        }
    }

    if( input_string.length() < min_len ) {
        SECTION("If min len is greater than input string length then string is padded") {
            REQUIRE(output_string[0] == ' ');
        }
    }
}


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

        auto generate(CatchKit::Detail::RandomNumberGenerator&) const -> int {
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