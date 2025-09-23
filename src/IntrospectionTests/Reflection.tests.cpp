//
// Created by Phil Nash on 23/09/2025.
//

#include "catch23/test.h"
#include "catchkit/matchers.h"

struct MyCustomType {};

inline CatchKit::Tag reflection_tag("reflection");

TEST( "Names can be obtained from simple, built-in, types", [reflection_tag] ) {
    CHECK(CatchKit::get_type_name<int>() == "int");
    CHECK(CatchKit::get_type_name<float>() == "float");
    CHECK(CatchKit::get_type_name<double>() == "double");
    CHECK(CatchKit::get_type_name<char>() == "char");
    CHECK(CatchKit::get_type_name<bool>() == "bool");
}
TEST( "Names can be obtained from simple, custom, types", [reflection_tag] ) {
    CHECK(CatchKit::get_type_name<MyCustomType>() == "MyCustomType");
}
TEST("Names can be obtained from pointers", [reflection_tag]) {
    // We have to be a bit careful because different compilers have different spacing around the *
    CHECK_THAT(CatchKit::get_type_name<int*>(), starts_with("int") && ends_with("*"));
    CHECK_THAT(CatchKit::get_type_name<char*>(), starts_with("char") && ends_with("*"));
    CHECK_THAT(CatchKit::get_type_name<MyCustomType*>(), starts_with("MyCustomType") && ends_with("*"));
}
TEST("Names can be obtained from pointers to const", [reflection_tag]) {
    // const may come before or after the type name (although most compilers put it before)
    CHECK_THAT(CatchKit::get_type_name<int const*>(), contains("int") && contains("const") && ends_with("*"));
    CHECK_THAT(CatchKit::get_type_name<char const*>(), contains("char") && contains("const") && ends_with("*"));
    CHECK_THAT(CatchKit::get_type_name<MyCustomType const*>(), contains("MyCustomType") && contains("const") && ends_with("*"));
}

TEST("pairs and tuples can be converted to strings", [reflection_tag]) {
    // tested separately because the `,` may confuse parsing
    CHECK(CatchKit::get_type_name<std::pair<int, int>>() == "std::pair<int, int>");
    CHECK(CatchKit::get_type_name<std::tuple<float, double, bool>>() == "std::tuple<float, double, bool>");
}

enum class Colours{ red, green, blue };

TEST("Enum classes can be converted to strings", [reflection_tag]) {
    CHECK( CatchKit::enum_to_string( Colours::red ) == "red");
    CHECK( CatchKit::enum_to_string( Colours::green ) == "green");
    CHECK( CatchKit::enum_to_string( Colours::blue ) == "blue");
}

enum ColoursUnscoped{ red, green, blue };
static_assert(requires { std::integral_constant<ColoursUnscoped, static_cast<ColoursUnscoped>(std::size_t())>{}; } );

TEST("Unscoped enums can be converted to strings", [reflection_tag]) {
    CHECK( CatchKit::enum_to_string( ColoursUnscoped::red ) == "red");
    CHECK( CatchKit::enum_to_string( ColoursUnscoped::green ) == "green");
    CHECK( CatchKit::enum_to_string( ColoursUnscoped::blue ) == "blue");
}
