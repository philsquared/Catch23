//
// Created by Phil Nash on 24/07/2025.
//

#ifdef USE_CATCH23_MODULES
    #include "catch23/macros.h"
    import catch23;
#else
    #include "catch23/meta_test.h"
    #include "catch23/test.h"
    #include "catchkit/matchers.h"
#endif

#include "catchkit/expression_info.h"

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
    CHECK( result1.failed() );
    auto expr1 = std::get_if<CatchKit::BinaryExpressionInfo>( &result1.expression_info );
    REQUIRE( expr1 );
    CHECK( expr1->lhs == "42" );
    CHECK( expr1->rhs == "54" );
    CHECK( expr1->op == "==" );

    auto const& result2 = results[1].info;
    CHECK( result2.message.empty() );
    CHECK( result2.passed() );
    auto expr2 = std::get_if<CatchKit::BinaryExpressionInfo>( &result2.expression_info );
    REQUIRE( expr2 );
    CHECK( expr2->lhs == "2" );
    CHECK( expr2->rhs == "2" );
    CHECK( expr2->op == "==" );
}

TEST( "Variables can be captured" ) {
    auto results = LOCAL_TEST() {
        int x = 7, y = 42;
        std::string s = "hello world";
        float f = 3.14;
        CAPTURE(x, y, s, f);

        FAIL();
    };

    REQUIRE( results.size() == 1 );
    auto const& result = results[0];
    CHECK( result.failed() );
    auto const& vars = result.info.variables;
    REQUIRE( vars.size() == 4 );
    CHECK( vars[0].name == "x" );
    CHECK( vars[0].type == "int" );
    CHECK( vars[0].value == "7" );

    CHECK( vars[1].name == "y" );
    CHECK( vars[1].type == "int" );
    CHECK( vars[1].value == "42" );

    CHECK( vars[2].name == "s" );
    CHECK( vars[2].type == "std::string" );
    CHECK( vars[2].value == "\"hello world\"" );

    CHECK( vars[3].name == "f" );
    CHECK( vars[3].type == "float" );
    CHECK( vars[3].value == "3.14" );
}

struct NonConstEqualsNonConstRef {
    auto operator==(NonConstEqualsNonConstRef&) { return true; }
};
struct NonConstEqualsValue {
    auto operator==(NonConstEqualsValue) { return true; }
};

// Note that this is not valid, so is not supported:
struct NonConstEqualsConstRef {
    auto operator==(NonConstEqualsConstRef const&) { return true; }
};

TEST("Types with non-const equality operators can be used") {
    SECTION("with non-const-ref arg") {
        NonConstEqualsNonConstRef nce;
        CHECK(nce ==nce);
    }
    SECTION("with value arg") {
        NonConstEqualsValue nce;
        CHECK(nce ==nce);
    }
}

