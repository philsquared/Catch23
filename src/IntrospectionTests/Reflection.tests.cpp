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

TEST("Invalid enum classes can be converted to strings", [reflection_tag]) {
    auto ultra_violet = static_cast<Colours>(static_cast<int>(Colours::blue)+1);
    CHECK( CatchKit::enum_to_string( ultra_violet ) == "(Colours)3");
}

enum ColoursUnscoped{ red, green, blue };
static_assert(requires { std::integral_constant<ColoursUnscoped, static_cast<ColoursUnscoped>(std::size_t())>{}; } );

TEST("Unscoped enums can be converted to strings", [reflection_tag]) {
    CHECK( CatchKit::enum_to_string( ColoursUnscoped::red ) == "red");
    CHECK( CatchKit::enum_to_string( ColoursUnscoped::green ) == "green");
    CHECK( CatchKit::enum_to_string( ColoursUnscoped::blue ) == "blue");
}

enum class BigEnum {
    Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
    Ten, Eleven, Twelve, Thirteen, Fourteen, Fifteen, Sixteen, Seventeen, Eighteen, Nineteen,
    Twenty, TwentyOne, TwentyTwo, TwentyThree, TwentyFour, TwentyFive, TwentySix, TwentySeven, TwentyEight, TwentyNine,
    Thirty, ThirtyOne, ThirtyTwo
};

TEST("Larger enum classes can be converted to strings", [reflection_tag]) {
    CHECK( CatchKit::enum_to_string( BigEnum::One ) == "One");
    CHECK( CatchKit::enum_to_string( BigEnum::Sixteen ) == "Sixteen");
    CHECK( CatchKit::enum_to_string( BigEnum::Seventeen ) == "Seventeen");
    CHECK( CatchKit::enum_to_string( BigEnum::ThirtyTwo ) == "ThirtyTwo");
}

enum class Negative {
    MinusOne = -1,
    Zero = 0,
    One = 1
};

TEST("Enums with negative values can be converted to strings", [reflection_tag]) {
    CHECK( CatchKit::enum_to_string( Negative::MinusOne ) == "MinusOne");
    CHECK( CatchKit::enum_to_string( Negative::Zero ) == "Zero");
    CHECK( CatchKit::enum_to_string( Negative::One ) == "One");
}

namespace Ns1::Ns2 {
    enum class Nested { a, b, c };
}
template<typename T> struct Tmpl { enum class E { V }; };

TEST("Some tricky enum conversions", [reflection_tag]) {
    SECTION("nested namespace") {
        CHECK( CatchKit::enum_to_string( Ns1::Ns2::Nested::b ) == "b" );
    }
    SECTION("Within template") {
        struct E{};
        CHECK( CatchKit::enum_to_string( Tmpl<E>::E::V ) == "V" );
    }
    SECTION("Defined inside a lambda") {
        auto lambda = [] {
            enum class Inner { V };
            return CatchKit::enum_to_string(Inner::V);
        };
        CHECK( lambda() == "V" );
    }
    SECTION("enums with aliases") {
        enum class Alias { A = 0, B = 0, C = 1 };

        CHECK_THAT( CatchKit::enum_to_string( Alias::A ), equals("A") || equals("B") );
        CHECK_THAT( CatchKit::enum_to_string( Alias::B ), equals("A") || equals("B") );
        CHECK( CatchKit::enum_to_string( Alias::C ) == "C" );
    }
    SECTION("Gaps spanning zero") {
        enum class Gappy { A = -10, B = 10 };
        CHECK( CatchKit::enum_to_string( Gappy::A ) == "A" );
        CHECK( CatchKit::enum_to_string( Gappy::B ) == "B" );
    }
    SECTION("Single enum") {
        enum class Single { Only = 10 };
        CHECK( CatchKit::enum_to_string( Single::Only ) == "Only" );
    }
    SECTION("Empty enum") {
        enum class Empty {};
        CHECK_THAT( CatchKit::enum_to_string(static_cast<Empty>(0)), contains("0") );
    }
    SECTION("Bitmask combinations") {
        enum class Flags { A = 1, B = 2, C = 4 };
        CHECK_THAT( CatchKit::enum_to_string(static_cast<Flags>(3)), contains("3") ); // A|B
    }
    SECTION("Tricky names") {
        enum class Names { _, _0, x, X, red, Red, RED };
        CHECK( CatchKit::enum_to_string( Names::_ ) == "_" );
        CHECK( CatchKit::enum_to_string( Names::_0 ) == "_0" );
        CHECK( CatchKit::enum_to_string( Names::x ) == "x" );
        CHECK( CatchKit::enum_to_string( Names::X ) == "X" );
        CHECK( CatchKit::enum_to_string( Names::red ) == "red" );
        CHECK( CatchKit::enum_to_string( Names::Red ) == "Red" );
        CHECK( CatchKit::enum_to_string( Names::RED ) == "RED" );
    }
    SECTION("Out of range") {
        // The sparse problem limit is 16. We can go beyond that sequentially,
        // but if there is a gap we stop.
        // If the sparse probe limit changes this test will need to be updated
        enum class Sparse { a=15, b=16, c=17, d=19 };
        CHECK( CatchKit::enum_to_string( Sparse::a ) == "a" );
        CHECK( CatchKit::enum_to_string( Sparse::b ) == "b" );
        CHECK( CatchKit::enum_to_string( Sparse::c ) == "c" );
        CHECK_THAT( CatchKit::enum_to_string( Sparse::d ), contains("19") );

        // The sequential probe limit is 64, so we can't convert anything past that
        enum class Seq {
            a=16, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
            aa, bb, cc, dd, ee, ff, gg, hh, ii, jj, kk, ll, mm, nn,
            oo, pp, qq, rr, ss, tt, uu, vv, ww, xx, yy, zz
        };

        CHECK( CatchKit::enum_to_string( Seq::a ) == "a" );
        CHECK( CatchKit::enum_to_string( Seq::b ) == "b" );
        CHECK( CatchKit::enum_to_string( Seq::ww ) == "ww" );
        CHECK_THAT( CatchKit::enum_to_string( Seq::xx ), contains("65") );
        CHECK_THAT( CatchKit::enum_to_string( Seq::yy ), contains("66") );
        CHECK_THAT( CatchKit::enum_to_string( Seq::zz ), contains("67") );
    }
    SECTION("Expression defined enums") {
        enum class Expr { A = 1 << 0, B = 1 << 1, C = 1 << 2 };
        CHECK( CatchKit::enum_to_string(Expr::A) == "A" );
        CHECK( CatchKit::enum_to_string(Expr::C) == "C" );
    }
    SECTION("Large underlying with big value") {
        enum class Big : uint64_t { Large = 1'000'000'000'000ULL };
        CHECK_THAT( CatchKit::enum_to_string(Big::Large), contains("1000000000000") );
    }
    SECTION("underlying type boundaries") {
        SECTION("default max probe") {
            enum class Tiny : int8_t { Min = -128, Max = 127 };
            CHECK_THAT( CatchKit::enum_to_string( Tiny::Min ), contains("-128") );
            CHECK_THAT( CatchKit::enum_to_string( Tiny::Max ), contains("127") );
        }
        SECTION("bigger max probe") {
            // Use version in Detail that lets us specify probe limits
            enum class Tiny : int8_t { Min = -128, Max = 127 };
            CHECK( CatchKit::probed_enum_to_string<128, 128>( Tiny::Min ) == "Min" );
            CHECK( CatchKit::probed_enum_to_string<128, 128>( Tiny::Max ) == "Max" );
        }
        SECTION("Bool underlying type") {
            enum class Bool : bool { False = false, True = true };
            CHECK( CatchKit::enum_to_string(Bool::False) == "False" );
            CHECK( CatchKit::enum_to_string(Bool::True) == "True" );
        }
        SECTION("unsigned underlying") {
            enum class Unsigned : uint8_t { A = 0, B = 255 };
            CHECK( CatchKit::enum_to_string(Unsigned::A) == "A" );
            CHECK( CatchKit::probed_enum_to_string<256, 256>(Unsigned::B) == "B" );
        }
    }
}