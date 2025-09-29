#include "catch23/test.h"
#include "catchkit/matchers.h"

TEST("Built-ins can be converted to strings") {
    CHECK( CatchKit::stringify( 0 ) == "0" );
    CHECK( CatchKit::stringify( 42 ) == "42" );
    CHECK( CatchKit::stringify( 0.1f ) == "0.1" );
    CHECK( CatchKit::stringify( static_cast<double>(1.23) ) == "1.23" );

    CHECK( CatchKit::stringify( "char*" ) == "\"char*\"" );
    CHECK( CatchKit::stringify( std::string("string") ) == "\"string\"" );

    int* p = nullptr;

    CHECK( CatchKit::stringify(p) == "nullptr" );

    int n = 123;
    p = &n;

    CHECK_THAT( CatchKit::stringify(p), starts_with("0x") && has_size(14) );
}
