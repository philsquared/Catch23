//
// Created by Phil Nash on 29/11/2025.
//

#include "catch23/clara.h"

#include "catch23/test_registry.h"
#ifdef USE_CATCH23_MODULES
    #include "catch23/macros.h"
    import catch23;
#else
#include "catch23/test.h"
// #include "catchkit/matchers.h"
#endif

using namespace CatchKit::Clara;

TEST("Single simple flag parsers") {
    bool simple = false;

    auto cli = Flag("-a --abc", "simple bool", simple);

    SECTION("No args passed") {
        cli.parse({});
        REQUIRE(simple == false);
    }
    SECTION("No args passed when required") {
        cli.required().parse({});
        REQUIRE(simple == false);
    }
    SECTION("Short arg passed") {
        cli.parse({"-a"});
        REQUIRE(simple == true);
    }
    SECTION("Long arg passed") {
        cli.parse({"--abc"});
        REQUIRE(simple == true);
    }
}

TEST("Composed flag parsers") {
    bool a = false;
    bool b = false;

    auto cli_a = Flag("-a --abc", "simple bool", a);
    auto cli_b = Flag("-b --bcd", "simple bool2", b);
    auto cli = cli_a | cli_b;

    SECTION("No args passed") {
        cli.parse({});
        REQUIRE(a == false);
        REQUIRE(b == false);
    }
    SECTION("-a passed") {
        cli.parse({"-a"});
        REQUIRE(a == true);
        REQUIRE(b == false);
    }
    SECTION("-b passed") {
        cli.parse({"-b"});
        REQUIRE(a == false);
        REQUIRE(b == true);
    }
    SECTION("-a and -b passed (separately)") {
        cli.parse({"-b", "-a"});
        REQUIRE(a == true);
        REQUIRE(b == true);
    }
    SECTION("-a and -b passed (together)") {
        cli.parse({"-ab"});
        REQUIRE(a == true);
        REQUIRE(b == true);
    }
    SECTION("Long args passed") {
        cli.parse({"--abc", "--bcd"});
        REQUIRE(a == true);
        REQUIRE(b == true);
    }
}

TEST("Single simple opt parser") {
    bool simple = false;

    auto cli
        = Opt("-a", "bool with no transformer", simple )
        | Opt("-b", "bool with transformer",
            [&](std::string_view value) -> std::expected<void, ParserError> {
                simple = true;
                return {};
            } );

    SECTION("No args passed") {
        cli.parse({});
        CHECK(simple == false);
    }
    SECTION("missing arg") {
        auto result = cli.parse({"-a"});
        CHECK(!result.has_value());
        CHECK(result.error() == ParserError::MissingArgument );
        CHECK(simple == false);
    }
    SECTION("Short bool arg passed (no transform)") {
        auto result = cli.parse({"-a", "yes"});
        CHECK(simple == false);
        REQUIRE_FALSE( result );
        CHECK( result.error() == ParserError::ConversionFailure );
    }
    SECTION("Short bool arg passed (transformed)") {
        CHECK(cli.parse({"-b", "yes"}));
        CHECK(simple == true);
    }
}
TEST("Single simple arg parser") {
    int a = 0;

    auto cli = Arg("arg", "a simple int", a);

    SECTION("No args passed") {
        cli.parse({});
        CHECK(a == 0);
    }
    SECTION("No args passed when required") {
        auto result = cli.required().parse({});
        CHECK( a == 0 );
        REQUIRE( !result );
        CHECK( result.error() == ParserError::MissingArgument );
    }
    SECTION("arg passed") {
        auto result = cli.parse({"42"});
        CHECK( result );
        CHECK( a == 42 );
    }
}
