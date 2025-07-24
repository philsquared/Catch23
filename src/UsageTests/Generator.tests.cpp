//
// Created by Phil Nash on 24/07/2025.
//

#include "catch23/catch23_test.h"
#include "catch23/catch23_generators.h"

TEST("Generators", "[.]") {
    auto i = GENERATE(100, values_of<int>());

    CHECK( i < 50 ); // Half of these should fail
}

TEST("Two generators", "[.]") {
    auto i = GENERATE(10, values_of<int>{ .up_to=10 });
    auto j = GENERATE(10, values_of<int>{ .up_to=10 });

    CHECK( i < j ); // Half of these should fail
}

TEST("string generators") {
    auto words = GENERATE(10, values_of<std::string>{} );
    auto symbols = GENERATE(10, values_of<std::string>{.charset=Charsets::symbols} );

    CHECK( words != symbols );
}
