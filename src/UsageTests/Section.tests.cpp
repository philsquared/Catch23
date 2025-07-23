//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23/catch23_test.h"
#include "catch23/catch23_sections.h"
#include "catch23/catch23_catch2.h"


TEST_CASE( "random SECTION tests", "[.][sections][failing]" ) {
    int a = 1;
    int b = 2;

    SECTION( "doesn't equal" ) {
        REQUIRE( a != b );
        REQUIRE( b != a );
    }

    SECTION( "not equal" ) {
        REQUIRE( a != b);
    }
}

TEST_CASE( "nested SECTION tests", "[.][sections][failing]" ) {
    int a = 1;
    int b = 2;

    SECTION( "doesn't equal" ) {
        REQUIRE( a != b );
        REQUIRE( b != a );

        SECTION( "not equal" ) {
            REQUIRE( a != b);
        }
    }
}

TEST_CASE( "more nested SECTION tests", "[sections][failing][/.]" ) {
    int a = 1;
    int b = 2;

    SECTION( "doesn't equal" ) {
        SECTION( "equal" ) {
            REQUIRE( a == b );
        }

        // !TBD node handling needs to account for early exits (as above)

        SECTION( "not equal" ) {
            REQUIRE( a != b );
        }
        SECTION( "less than" ) {
            REQUIRE( a < b );
        }
    }
}