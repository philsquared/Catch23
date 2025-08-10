//
// Created by Phil Nash on 10/08/2025.
//

#include "catch23/test.h"
#include "catch23/generators.h"
#include "catch23/local_test.h"

TEST("Two PBT generators", "[/.]") {
    auto i = GENERATE(10, values_of<int>{ .up_to=16 });
    auto j = GENERATE(10, values_of<int>{ .up_to=32 });

    CAPTURE( i, j );
    if( (i == 3 || i > 11) && j > 4 )
        FAIL();
    else
        PASS();
}