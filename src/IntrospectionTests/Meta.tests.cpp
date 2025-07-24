//
// Created by Phil Nash on 24/07/2025.
//

#include "catch23/catch23_local_test.h"
#include "catch23/catch23_test.h"

#include "catchkit/catchkit_matchers.h"


TEST("A test that can run tests") {

    auto results = LOCAL_TEST() {
        CHECK(42==6*9);
        CHECK(2==2);
    };

    REQUIRE( results.size() == 2 );

    auto const& context = results[0].context;
    CHECK( context.macro_name == "CHECK" );
    CHECK_THAT( context.original_expression, contains("6*9") );

    auto const& result1 = results[0].info;
    CHECK( result1.message.empty() );
    CHECK( !result1.passed() );
    REQUIRE( result1.expression_info );
    CHECK( result1.expression_info->lhs == "42" );
    CHECK( result1.expression_info->rhs == "54" );
    CHECK( result1.expression_info->op == CatchKit::Detail::Operators::Equals );

    auto const& result2 = results[1].info;
    CHECK( result2.message.empty() );
    CHECK( result2.passed() );
    REQUIRE( result2.expression_info );
    CHECK( result2.expression_info->lhs == "2" );
    CHECK( result2.expression_info->rhs == "2" );
    CHECK( result2.expression_info->op == CatchKit::Detail::Operators::Equals );

}
