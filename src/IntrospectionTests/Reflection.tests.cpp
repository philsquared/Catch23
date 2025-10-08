//
// Created by Phil Nash on 23/09/2025.
//

#ifdef USE_CATCH23_MODULES
    #include "catch23/macros.h"
    import catch23;
#else
    #include "catch23/test.h"
    #include "catchkit/matchers.h"
#endif

#include "catchkit/reflection.h"

struct MyCustomType {};

inline CatchKit::Tag reflection_tag("reflection");

TEST( "Names can be obtained from simple, built-in, types", [reflection_tag] ) {
    CHECK(CatchKit::type_to_string<int>() == "int");
    CHECK(CatchKit::type_to_string<float>() == "float");
    CHECK(CatchKit::type_to_string<double>() == "double");
    CHECK(CatchKit::type_to_string<char>() == "char");
    CHECK(CatchKit::type_to_string<bool>() == "bool");
}
TEST( "Names can be obtained from simple, custom, types", [reflection_tag] ) {
    CHECK(CatchKit::type_to_string<MyCustomType>() == "MyCustomType");
}
TEST("Names can be obtained from pointers", [reflection_tag]) {
    // We have to be a bit careful because different compilers have different spacing around the *
    CHECK_THAT(CatchKit::type_to_string<int*>(), starts_with("int") && ends_with("*"));
    CHECK_THAT(CatchKit::type_to_string<char*>(), starts_with("char") && ends_with("*"));
    CHECK_THAT(CatchKit::type_to_string<MyCustomType*>(), starts_with("MyCustomType") && ends_with("*"));
}
TEST("Names can be obtained from pointers to const", [reflection_tag]) {
    // const may come before or after the type name (although most compilers put it before)
    CHECK_THAT(CatchKit::type_to_string<int const*>(), contains("int") && contains("const") && ends_with("*"));
    CHECK_THAT(CatchKit::type_to_string<char const*>(), contains("char") && contains("const") && ends_with("*"));
    CHECK_THAT(CatchKit::type_to_string<MyCustomType const*>(), contains("MyCustomType") && contains("const") && ends_with("*"));
}

TEST("pairs and tuples can be converted to strings", [reflection_tag]) {
    // tested separately because the `,` may confuse parsing
    CHECK(CatchKit::type_to_string<std::pair<int, int>>() == "std::pair<int, int>");
    CHECK(CatchKit::type_to_string<std::tuple<float, double, bool>>() == "std::tuple<float, double, bool>");
}

TEST("std alias type names are normalised", [reflection_tag]) {
    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::string>() ) == "std::string" );
    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::string_view>() ) == "std::string_view" );

    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::wstring>() ) == "std::wstring" );
    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::wstring_view>() ) == "std::wstring_view" );

    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::u8string>() ) == "std::u8string" );
    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::u8string_view>() ) == "std::u8string_view" );

    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::u16string>() ) == "std::u16string" );
    CHECK( CatchKit::Detail::normalise_type_name( CatchKit::type_to_string<std::u16string_view>() ) == "std::u16string_view" );
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
