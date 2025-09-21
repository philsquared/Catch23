//
// Created by Phil Nash on 10/08/2025.
//

#include "catch23/test.h"
#include "catch23/generators.h"
#include "catch23/meta_test.h"

TEST("Single int generator", [mute]) {
     auto i = GENERATE(values_of<int>{ .up_to=16 });

    CAPTURE( i );
    CHECK( (i == 3 || i > 11 ) );
}

TEST("Two PBT generators", [mute, "tag"]) {
    auto i = GENERATE(10, values_of<int>{ .up_to=16 });
    auto j = GENERATE(10, values_of<int>{ .up_to=32 });

    CAPTURE( i, j );
    if( (i == 3 || i > 11) && j > 5 )
        FAIL();
    else
        PASS();
}

std::pair<int, int> sd(int N) {
    int a = N & 0b0101010101010101;
    int b = N & 0b1010101010101010;
    return {a, b};
}

// This is based on an actual Codility exercise
TEST("sparse decomposition", [mute]) {
    int N = GENERATE(values_of<int>());
    auto&& [a, b] = sd(N);

    CHECK( a+b == N);
}
