//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23/test.h"
#include "catch23/catch2_compat.h"


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

TEST_CASE( "more nested SECTION tests", "[sections][failing][.]" ) {
    // check.result_handler.report_on = CatchKit::ReportOn::AllResults;

    int a = 1;
    int b = 2;

    SECTION( "doesn't equal" ) {
        SECTION( "equal" ) {
            REQUIRE( a == b );
        }

        SECTION( "not equal" ) {
            REQUIRE( a != b );
        }
        SECTION( "less than" ) {
            REQUIRE( a < b );
        }
    }
}

TEST_CASE( "even more nested SECTION tests", "[sections]" ) {
    SECTION( "c" ) {
        SECTION( "d (leaf)" ) {
            SUCCEED(); // avoid failing due to no tests
        }

        SECTION( "e (leaf)" ) {
            SUCCEED(); // avoid failing due to no tests
        }
    }

    SECTION( "f (leaf)" ) {
        SUCCEED(); // avoid failing due to no tests
    }
}

TEST_CASE( "looped SECTION tests", "[.][failing][sections]" ) {
    int a = 2;

    for( int b = 0; b < 10; ++b ) {
        SECTION( std::format("b is currently: {}", b ) ) {
            CHECK( b > a );
        }
    }
}

TEST_CASE( "looped tests", "[.][failing]" ) {
    static const int fib[]  = { 1, 1, 2, 3, 5, 8, 13, 21 };

    for( std::size_t i=0; i < sizeof(fib)/sizeof(int); ++i ) {
        CHECK( ( fib[i] % 2 ) == 0,
            std::format("Testing if fib[{}] ({}) is even", i, fib[i] ) );
    }
}

TEST_CASE( "vectors can be sized and resized", "[vector]" ) {

    std::vector<int> v( 5 );

    REQUIRE( v.size() == 5 );
    REQUIRE( v.capacity() >= 5 );

    SECTION( "resizing bigger changes size and capacity" ) {
        v.resize( 10 );

        REQUIRE( v.size() == 10 );
        REQUIRE( v.capacity() >= 10 );
    }
    SECTION( "resizing smaller changes size but not capacity" ) {
        v.resize( 0 );

        REQUIRE( v.size() == 0 );
        REQUIRE( v.capacity() >= 5 );

        SECTION( "We can use the 'swap trick' to reset the capacity" ) {
            std::vector<int> empty;
            empty.swap( v );

            REQUIRE( v.capacity() == 0 );
        }
    }
    SECTION( "reserving bigger changes capacity but not size" ) {
        v.reserve( 10 );

        REQUIRE( v.size() == 5 );
        REQUIRE( v.capacity() >= 10 );
    }
    SECTION( "reserving smaller does not change size or capacity" ) {
        v.reserve( 0 );

        REQUIRE( v.size() == 5 );
        REQUIRE( v.capacity() >= 5 );
    }
}