//
// Created by Phil Nash on 24/07/2025.
//

#include "catch23/catch23_test.h"
#include "catch23/catch23_generators.h"

TEST("Generators") {
    auto i = GENERATE(100 * values_of<int>());

    CHECK( i < 50 ); // Half of these should fail
}