//
// Created by Phil Nash on 23/09/2025.
//

#ifdef USE_CATCH23_MODULES
    #include "catch23/catch23_mod.h"
#else
    #include "catch23/catch2_compat.h"
#endif

#include <utility>
#include <string>
#include <vector>

// Based on the same file in the Catch2 codebase ---

TEST_CASE( "std::pair<int,std::string> -> toString", "[toString][pair]" ) {
    std::pair<int,std::string> value( 34, "xyzzy" );
    REQUIRE( ::CatchKit::stringify( value ) == "(34, \"xyzzy\")" );
}

TEST_CASE( "std::pair<int,const std::string> -> toString", "[toString][pair]" ) {
    std::pair<int,const std::string> value( 34, "xyzzy" );
    REQUIRE( ::CatchKit::stringify(value) == "(34, \"xyzzy\")" );
}

TEST_CASE( "std::vector<std::pair<std::string,int> > -> toString", "[toString][pair]" ) {
    std::vector<std::pair<std::string,int> > pr;
    pr.push_back( std::make_pair("green", 55 ) );
    REQUIRE( ::CatchKit::stringify( pr ) == "[(\"green\", 55)]" );
}

// This is pretty contrived - I figure if this works, anything will...
TEST_CASE( "pair<pair<int,const char *,pair<std::string,int> > -> toString", "[toString][pair]" ) {
    typedef std::pair<int,const char *> left_t;
    typedef std::pair<std::string,int> right_t;

    left_t  left( 42, "Arthur" );
    right_t right( "Ford", 24 );

    std::pair<left_t,right_t> pair( left, right );
    REQUIRE( ::CatchKit::stringify( pair ) == "((42, \"Arthur\"), (\"Ford\", 24))" );
}
