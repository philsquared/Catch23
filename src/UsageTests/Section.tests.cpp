//
// Created by Phil Nash on 23/07/2025.
//

#ifdef USE_CATCH23_MODULES
    #include "catch23/catch23_mod.h"
#else
    #include "catch23/test.h"
    #include "catch23/meta_test.h"

    #include "catch23/internal_test.h"
#endif

#include "catch23/catch2_compat.h"

#include <iostream>
#include <ostream>

TEST_CASE( "random SECTION tests", "[.][sections][failing]" ) {
    int a = 1;
    int b = 2;

    SECTION( "doesn't equal" ) {
        REQUIRE( a != b );
        REQUIRE( b != a );
    }
    SECTION( "does equal" ) { // These should fail
        CHECK( a == b );
        CHECK( b == a );
    }

    SECTION( "not equal" ) {
        REQUIRE( a != b);
    }
    SECTION( "equal" ) { // This should fail
        REQUIRE( a == b);
    }
}
TEST("Meta: random SECTION tests", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME("random SECTION tests").failures() == 3 );
}

TEST_CASE( "nested SECTION tests", "[.][sections][failing]" ) {
    int a = 1;
    int b = 2;

    SECTION( "both equal" ) {
        CHECK( a == b );
        CHECK( b == a );

        SECTION( "equal" ) {
            REQUIRE( a == b);
        }
    }
}
TEST("Meta: nested SECTION tests", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME("nested SECTION tests").failures() == 5 );
}

TEST_CASE( "more nested SECTION tests", "[sections][failing][.]" ) {
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
TEST("Meta: more nested SECTION tests", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME("more nested SECTION tests").failures() == 1 );
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
TEST( "Meta: looped SECTION tests", ["meta"] ) {
    CHECK( RUN_TEST_BY_NAME( "looped SECTION tests" ).failures() == 1 );
}

TEST_CASE( "looped tests", "[.][failing]" ) {
    static const int fib[]  = { 1, 1, 2, 3, 5, 8, 13, 21 };

    for( std::size_t i=0; i < sizeof(fib)/sizeof(int); ++i ) {
        CHECK( ( fib[i] % 2 ) == 0,
            std::format("Testing if fib[{}] ({}) is even", i, fib[i] ) );
    }
}
TEST( "Meta: looped tests", ["meta"] ) {
    CHECK( RUN_TEST_BY_NAME( "looped tests" ).failures() == 6 );
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