//
// Created by Phil Nash on 22/07/2025.
//

#ifdef USE_CATCH23_MODULES
    #include "catch23/catch23_mod.h"
#else

#include <catch23/catch2_compat.h>
#include <catchkit/matchers.h>

#include "catch23/meta_test.h"

#endif

#include <algorithm>
#include <exception>
#include <cmath>
#include <list>
#include <sstream>

void non_throwing_function() {}
void throwing_function(std::string const& message = {}) {
    throw std::domain_error( message );
}

TEST("Bound matchers", [mute]) {

    // These tests are for the matcher binding mechanism.
    // For exception matching in general see the subsequent tests

    // Using with_message implements the bound matcher internally:
    CHECK_THAT( throw std::domain_error("Get the message"), throws<std::domain_error>().with_message("Don't get the message") );

    // You can write a bound matcher manually by composing with >>=
    CHECK_THAT( throw std::domain_error("Get the message"), throws<std::domain_error>() >>= CatchKit::ExceptionMatchers::HasMessage("Get the message") );

    // You can compose multiple bound matchers with >>=
    CHECK_THAT( throw std::domain_error("Get the message"),
                    throws<std::domain_error>()
                        >>= CatchKit::ExceptionMatchers::HasMessage()
                        >>= contains("message2") );

    // The HasMessage matcher takes a value, so doesn't have to be composed with throws<>
    std::domain_error err("on the stack");
    CHECK_THAT( err, CatchKit::ExceptionMatchers::HasMessage() >>= contains("stack") );

    // The following shouldn't compile because equals() is not a bindable matcher
    // CHECK_THAT( "err", equals("err") >>= contains("stack") );
}

TEST("Meta: Bound matchers", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "Bound matchers" ).failures() == 2 );
}

struct UnknownType {};

TEST("throws matcher") {
    SECTION("throws() matches if the expression throws anything")
        CHECK_THAT(throwing_function(), throws() );

    SECTION("throws<E>() matches if the expression throws a particular type") {
        CHECK_THAT(throwing_function(), throws<std::domain_error>() );
    }
    SECTION("throws().with_message(message) matches if the expression throws an exception with a particular message") {
        CHECK_THAT(throwing_function("hello"), throws().with_message("hello") );
    }
    SECTION("throws().with_message(message) matches with 'unknown exception type' if the exception type is unknown") {
        CHECK_THAT( throw UnknownType(), throws().with_message("<unknown exception type>") );
    }
    SECTION("HasMessage() matches with 'unknown exception type' if an unknown exception type is applied directly") {
        CHECK_THAT( UnknownType(), CatchKit::ExceptionMatchers::HasMessage("<unknown exception type>") );
    }
}

TEST("!throws matcher succeeds when call doesn't throw") {
    CHECK_THAT( non_throwing_function(), !throws() );
}
TEST("!throws matcher fails when call does throw", [mute]) {
    CHECK_THAT( throwing_function(), !throws() );
}

TEST("Meta: !throws matcher fails when call does throw", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "!throws matcher fails when call does throw" ).failures() == 1 );
}

TEST("Composed matchers cannot be stored") {
    using namespace CatchKit::Matchers;
    auto is_close_to_zero = is_close_to(0.0, 0.1);
    auto is_zero = is_close_to(0.0, 0.0);
    CHECK_THAT( 0.01, is_close_to_zero && !is_zero ); // ok

    // The following should not compile if uncommented:
    // auto is_close_to_but_not_zero = is_close_to(0.0, 0.1) && !is_close_to(0.0, 0.0);
    // CHECK_THAT( 0.01, is_close_to_but_not_zero ); // dangling refs
    // CHECK_THAT( 0.01, is_close_to_but_not_zero && is_zero );
}

// The following tests have been taken from the Catch2 test suite,
// with modifications to allow for the new matcher syntax and semantics


#ifdef __clang__
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wweak-vtables"
#    pragma clang diagnostic ignored "-Wpadded"
#endif

namespace {

    static const char* testStringForMatching() {
        return "this string contains 'abc' as a substring";
    }

    static const char* testStringForMatching2() {
        return "some completely different text that contains one common word";
    }

    static bool alwaysTrue( int ) { return true; }
    static bool alwaysFalse( int ) { return false; }

#ifdef _MSC_VER
#    pragma warning( disable : 4702 ) // Unreachable code -- MSVC 19 (VS 2015)
                                      // sees right through the indirection
#endif

    struct SpecialException : std::exception {
        SpecialException( int i_ ): i( i_ ) {}

        char const* what() const noexcept override {
            return "SpecialException::what";
        }

        int i;
    };

    struct DerivedException : std::exception {
        char const* what() const noexcept override {
            return "DerivedException::what";
        }
    };

    static void doesNotThrow() {}

    [[noreturn]] static void throwsSpecialException( int i ) {
        throw SpecialException{ i };
    }

    [[noreturn]] static void throwsAsInt( int i ) { throw i; }

    [[noreturn]] static void throwsDerivedException() {
        throw DerivedException{};
    }

    class ExceptionMatcher {
        int m_expected;

    public:
        ExceptionMatcher( int i ): m_expected( i ) {}

        auto match( SpecialException const& se ) const -> CatchKit::MatchResult {
            return se.i == m_expected;
        }

        auto describe() const -> CatchKit::MatcherDescription {
            std::ostringstream ss;
            ss << "special exception has value of " << m_expected;
            return ss.str();
        }
    };

#ifdef __DJGPP__
    static float nextafter( float from, float to ) {
        return ::nextafterf( from, to );
    }

    static double nextafter( double from, double to ) {
        return ::nextafter( from, to );
    }
#else
    using std::nextafter;
#endif

} // end unnamed namespace

TEST_CASE( "String matchers", "[matchers]" ) {
    REQUIRE_THAT( testStringForMatching(), contains( "string" ) );
    REQUIRE_THAT( testStringForMatching(), contains<CaseInsensitive>( "string" ) );
    CHECK_THAT( testStringForMatching(), contains( "abc" ) );
    CHECK_THAT( testStringForMatching(), contains<CaseInsensitive>( "aBC" ) );

    CHECK_THAT( testStringForMatching(), starts_with( "this" ) );
    CHECK_THAT( testStringForMatching(), starts_with<CaseInsensitive>( "THIS" ) );
    CHECK_THAT( testStringForMatching(), ends_with( "substring" ) );
    CHECK_THAT( testStringForMatching(), ends_with<CaseInsensitive>( " SuBsTrInG" ) );
}

TEST_CASE( "Contains string matcher", "[.][failing][matchers]" ) {
    CHECK_THAT( testStringForMatching(),
                contains<CaseInsensitive>( "not there" ) );
    CHECK_THAT( testStringForMatching(), contains( "STRING" ) );
}
TEST("Meta: Contains string matcher", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "Contains string matcher" ).failures() == 2 );
}
TEST_CASE( "StartsWith string matcher", "[.][failing][matchers]" ) {
    CHECK_THAT( testStringForMatching(), starts_with( "This String" ) );
    CHECK_THAT( testStringForMatching(), starts_with<CaseInsensitive>( "string" ) );
}
TEST("Meta: StartsWith string matcher", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "StartsWith string matcher" ).failures() == 2 );
}

TEST_CASE( "EndsWith string matcher", "[.][failing][matchers]" ) {
    CHECK_THAT( testStringForMatching(), ends_with( "Substring" ) );
    CHECK_THAT( testStringForMatching(), ends_with<CaseInsensitive>( "this" ) );
}
TEST("Meta: EndsWith string matcher", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "EndsWith string matcher" ).failures() == 2 );
}

TEST_CASE( "Equals string matcher", "[.][failing][matchers]" ) {
    CHECK_THAT( testStringForMatching(), equals( "this string contains 'ABC' as a substring" ) );
    CHECK_THAT( testStringForMatching(), equals<CaseInsensitive>( "something else" ) );
}
TEST("Meta: Equals string matcher", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "Equals string matcher" ).failures() == 2 );
}

TEST_CASE( "Equals", "[matchers]" ) {
    CHECK_THAT( testStringForMatching(), equals( "this string contains 'abc' as a substring" ) );
    CHECK_THAT( testStringForMatching(),
                equals<CaseInsensitive>( "this string contains 'ABC' as a substring" ) );
}

TEST_CASE( "Regex string matcher -- libstdc++-4.8 workaround",
           "[matchers][approvals]" ) {
// DJGPP has similar problem with its regex support as libstdc++ 4.8
#ifndef __DJGPP__
    REQUIRE_THAT( testStringForMatching(),
                  matches_regex( "this string contains 'abc' as a substring" ) );
    REQUIRE_THAT( testStringForMatching(),
                  matches_regex<CaseInsensitive>( "this string CONTAINS 'abc' as a substring" ) );
    REQUIRE_THAT( testStringForMatching(),
                  matches_regex( "^this string contains 'abc' as a substring$" ) );
    REQUIRE_THAT( testStringForMatching(), matches_regex( "^.* 'abc' .*$" ) );
    REQUIRE_THAT( testStringForMatching(),
                  matches_regex<CaseInsensitive>( "^.* 'ABC' .*$" ) );
#endif

    REQUIRE_THAT( testStringForMatching2(),
                  !matches_regex( "this string contains 'abc' as a substring" ) );
}

TEST_CASE( "Regex string matcher", "[matchers][.failing]" ) {
    CHECK_THAT( testStringForMatching(),
                matches_regex( "this STRING contains 'abc' as a substring" ) );
    CHECK_THAT( testStringForMatching(),
                matches_regex( "contains 'abc' as a substring" ) );
    CHECK_THAT( testStringForMatching(),
                matches_regex( "this string contains 'abc' as a" ) );
}

TEST_CASE( "Matchers can be composed with the && operator",
           "[matchers][operators][operator&&]" ) {
    CHECK_THAT( testStringForMatching(),
                contains( "string" ) && contains( "abc" ) &&
                contains( "substring" ) && contains( "contains" ) );
}

TEST_CASE( "Matchers can be composed with the || operator",
           "[matchers][operators][operator||]" ) {
    CHECK_THAT( testStringForMatching(),
                contains( "string" ) || contains( "different" ) || contains( "random" ) );
    CHECK_THAT( testStringForMatching2(),
                contains( "string" ) || contains( "different" ) || contains( "random" ) );
}

TEST_CASE( "Matchers can be composed with both && and ||",
           "[matchers][operators][operator||][operator&&]" ) {
    CHECK_THAT( testStringForMatching(),
                ( contains( "string" ) || contains( "different" ) ) && contains( "substring" ) );
}

TEST_CASE( "Matchers can be composed with both && and || - failing",
           "[matchers][operators][operator||][operator&&][.failing]" ) {
    CHECK_THAT( testStringForMatching(),
                ( contains( "string" ) || contains( "different" ) ) && contains( "random" ) );
}
TEST("Meta: Matchers can be composed with both && and || - failing", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "Matchers can be composed with both && and || - failing" ).failures() == 1 );
}

TEST_CASE( "Matchers can be negated (Not) with the ! operator",
           "[matchers][operators][not]" ) {
    CHECK_THAT( testStringForMatching(), !contains( "different" ) );
}

TEST_CASE( "Matchers can be negated (Not) with the ! operator - failing",
           "[matchers][operators][not][.failing]" ) {
    CHECK_THAT( testStringForMatching(), !contains( "substring" ) );
}
TEST("Meta: Matchers can be negated (Not) with the ! operator - failing", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "Matchers can be negated (Not) with the ! operator - failing" ).failures() == 1 );
}

template <typename T> struct CustomAllocator : private std::allocator<T> {
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using value_type = T;

    template <typename U> struct rebind { using other = CustomAllocator<U>; };

    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    CustomAllocator() = default;

    CustomAllocator( const CustomAllocator& other ):
        std::allocator<T>( other ) {}

    template <typename U> CustomAllocator( const CustomAllocator<U>& ) {}

    ~CustomAllocator() = default;

    using std::allocator<T>::allocate;
    using std::allocator<T>::deallocate;
};

TEST_CASE( "Vector matchers", "[matchers][vector]" ) {
    std::vector<int> v;
    v.push_back( 1 );
    v.push_back( 2 );
    v.push_back( 3 );

    std::vector<int> v2;
    v2.push_back( 1 );
    v2.push_back( 2 );

    std::vector<double> v3;
    v3.push_back( 1 );
    v3.push_back( 2 );
    v3.push_back( 3 );

    std::vector<double> v4;
    v4.push_back( 1 + 1e-8 );
    v4.push_back( 2 + 1e-8 );
    v4.push_back( 3 + 1e-8 );

    std::vector<int, CustomAllocator<int>> v5;
    v5.push_back( 1 );
    v5.push_back( 2 );
    v5.push_back( 3 );

    std::vector<int, CustomAllocator<int>> v6;
    v6.push_back( 1 );
    v6.push_back( 2 );

    std::vector<int> empty;

    // !TBD: contains(str)

    SECTION( "Contains (element)" ) {
        CHECK_THAT( v, contains( 1 ) );
    }
    SECTION( "Contains (vector)" ) {
        CHECK_THAT( v, contains( v2 ) );
        CHECK_THAT( v, contains( std::vector{ 1, 2 } ) );

        v2.push_back( 3 ); // now exactly matches
        CHECK_THAT( v, contains( v2 ) );

        CHECK_THAT( v, contains( empty ) );
        CHECK_THAT( empty, contains( empty ) );

        CHECK_THAT( v5, contains( v2 ) );
        CHECK_THAT( v5, contains( v6 ) );
    }
    SECTION( "Contains (element), composed" ) {
        CHECK_THAT( v, contains( 1 ) && contains( 2 ) );
    }

    SECTION( "Equals" ) {

        // Same vector
        CHECK_THAT( v, equals( v ) );

        CHECK_THAT( empty, equals( empty ) );

        // Different vector with same elements
        CHECK_THAT( v, equals( std::vector{ 1, 2, 3 } ) );
        v2.push_back( 3 );
        CHECK_THAT( v, equals(v2) );

        // Different allocators
        CHECK_THAT( v5, equals(v2) );

        v6.push_back( 3 );
        CHECK_THAT( v5, equals(v6) );
    }
    SECTION( "UnorderedEquals" ) {
        CHECK_THAT( v, equals<InAnyOrder>( v ) );
        CHECK_THAT( v, equals<InAnyOrder>( std::vector{ 3, 2, 1 } ) );
        CHECK_THAT( empty, equals<InAnyOrder>( empty ) );

        auto permuted = v;
        std::next_permutation( begin( permuted ), end( permuted ) );
        REQUIRE_THAT( permuted, equals<InAnyOrder>( v ) );

        std::reverse( begin( permuted ), end( permuted ) );
        REQUIRE_THAT( permuted, equals<InAnyOrder>( v ) );

        CHECK_THAT( v5, equals<InAnyOrder>( permuted ) );

        auto v5_permuted = v5;
        std::next_permutation( begin( v5_permuted ), end( v5_permuted ) );
        CHECK_THAT( v5_permuted, equals<InAnyOrder>( v5 ) );
    }
}

TEST_CASE( "Vector matchers that fail", "[matchers][vector][.][failing]" ) {
    std::vector<int> v;
    v.push_back( 1 );
    v.push_back( 2 );
    v.push_back( 3 );

    std::vector<int> v2;
    v2.push_back( 1 );
    v2.push_back( 2 );

    std::vector<double> v3;
    v3.push_back( 1 );
    v3.push_back( 2 );
    v3.push_back( 3 );

    std::vector<double> v4;
    v4.push_back( 1.1 );
    v4.push_back( 2.1 );
    v4.push_back( 3.1 );

    std::vector<int> empty;

    SECTION( "Contains (element)" ) {
        CHECK_THAT( v, contains( -1 ) );
        CHECK_THAT( empty, contains( 1 ) );
    }
    SECTION( "Contains (vector)" ) {
        CHECK_THAT( empty, contains( v ) );
        v2.push_back( 4 );
        CHECK_THAT( v, contains( v2 ) );
    }

    SECTION( "Equals" ) {
        CHECK_THAT( v, equals( v2 ) );
        CHECK_THAT( v2, equals( v ) );
        CHECK_THAT( empty, equals( v ) );
        CHECK_THAT( v, equals( empty ) );
    }
    SECTION( "UnorderedEquals" ) {
        CHECK_THAT( v, equals<InAnyOrder>( empty ) );
        CHECK_THAT( empty, equals<InAnyOrder>( v ) );

        auto permuted = v;
        std::next_permutation( begin( permuted ), end( permuted ) );
        permuted.pop_back();
        CHECK_THAT( permuted, equals<InAnyOrder>( v ) );

        std::reverse( begin( permuted ), end( permuted ) );
        CHECK_THAT( permuted, equals<InAnyOrder>( v ) );
    }
}

namespace {
    struct SomeType {
        int i;
        friend bool operator==( SomeType lhs, SomeType rhs ) {
            return lhs.i == rhs.i;
        }
    };
} // end anonymous namespace

TEST_CASE( "Vector matcher with elements without !=", "[matchers][vector][approvals]" ) {
    std::vector<SomeType> lhs, rhs;
    lhs.push_back( { 1 } );
    lhs.push_back( { 2 } );
    rhs.push_back( { 1 } );
    rhs.push_back( { 1 } );

    REQUIRE_THAT( lhs, !equals(rhs) );
}

TEST_CASE( "Exception matchers that succeed",
           "[matchers][exceptions][!throws]" ) {
    CHECK_THAT( throwsSpecialException( 1 ), throws<SpecialException>() >>= ExceptionMatcher{ 1 } );
    REQUIRE_THAT( throwsSpecialException( 2 ), throws<SpecialException>() >>= ExceptionMatcher{ 2 } );
}

TEST_CASE( "Exception matchers that fail",
           "[matchers][exceptions][!throws][.failing]" ) {
    SECTION( "No exception" ) {
        CHECK_THAT( doesNotThrow(), throws<SpecialException>() >>= ExceptionMatcher{ 1 } );
        REQUIRE_THAT( doesNotThrow(), throws<SpecialException>() >>= ExceptionMatcher{ 1 } );
    }
    SECTION( "Type mismatch" ) {
        CHECK_THAT( throwsAsInt( 1 ), throws<SpecialException>() >>= ExceptionMatcher{ 1 } );
        REQUIRE_THAT( throwsAsInt( 1 ), throws<SpecialException>() >>= ExceptionMatcher{ 1 } );
    }
    SECTION( "Contents are wrong" ) {
        CHECK_THAT( throwsSpecialException( 3 ), throws<SpecialException>() >>= ExceptionMatcher{ 1 } );
        REQUIRE_THAT( throwsSpecialException( 4 ), throws<SpecialException>() >>= ExceptionMatcher{ 1 } );
    }
}
TEST("Meta: Exception matchers that fail", ["meta"]) {
    CHECK( RUN_TEST_BY_NAME( "Exception matchers that fail" ).failures() == 6 );
}

TEST_CASE( "Floating point matchers: float", "[matchers][floating-point]" ) {
     SECTION( "Relative" ) {
         REQUIRE_THAT( 10.f, is_close_to_rel( 11.1f, 0.1f ) );
         REQUIRE_THAT( 10.f, !is_close_to_rel( 11.2f, 0.1f ) );
         REQUIRE_THAT( 1.f, !is_close_to_rel( 0.f, 0.99f ) );
         REQUIRE_THAT( -0.f, is_close_to_rel( 0.f ) );
         SECTION( "Some subnormal values" ) {
             auto v1 = std::numeric_limits<float>::min();
             auto v2 = v1;
             for ( int i = 0; i < 5; ++i ) {
                 v2 = std::nextafter( v1, 0.f );
             }
             REQUIRE_THAT( v1, is_close_to_rel( v2 ) );
         }
     }
     SECTION( "Margin" ) {
         REQUIRE_THAT( 1.f, is_close_to( 1.f, 0 ) );
         REQUIRE_THAT( 0.f, is_close_to( 1.f, 1 ) );

         REQUIRE_THAT( 0.f, !is_close_to( 1.f, 0.99f ) );
         REQUIRE_THAT( 0.f, !is_close_to( 1.f, 0.99f ) );

         REQUIRE_THAT( 0.f, is_close_to( -0.f, 0 ) );

         REQUIRE_THAT( 11.f, !is_close_to( 10.f, 0.5f ) );
         REQUIRE_THAT( 10.f, !is_close_to( 11.f, 0.5f ) );
         REQUIRE_THAT( -10.f, is_close_to( -10.f, 0.5f ) );
         REQUIRE_THAT( -10.f, is_close_to( -9.6f, 0.5f ) );
     }
     SECTION( "ULPs" ) {
         REQUIRE_THAT( 1.f, is_within_ulp( 1.f, 0 ) );
         REQUIRE_THAT(-1.f, is_within_ulp( -1.f, 0 ) );

         REQUIRE_THAT( nextafter( 1.f, 2.f ), is_within_ulp( 1.f, 1 ) );
         REQUIRE_THAT( 0.f, is_within_ulp( nextafter( 0.f, 1.f ), 1 ) );
         REQUIRE_THAT( 1.f, is_within_ulp( nextafter( 1.f, 0.f ), 1 ) );
         REQUIRE_THAT( 1.f, !is_within_ulp( nextafter( 1.f, 2.f ), 0 ) );

         REQUIRE_THAT( 1.f, is_within_ulp( 1.f, 0 ) );
         REQUIRE_THAT( -0.f, is_within_ulp( 0.f, 0 ) );
     }
     SECTION( "Composed" ) {
         REQUIRE_THAT( 1.f, is_close_to( 1.f, 0.5 ) || is_within_ulp( 1.f, 1 ) );
         REQUIRE_THAT( 1.f, is_close_to( 2.f, 0.5 ) || is_within_ulp( 1.f, 0 ) );
         REQUIRE_THAT( 0.0001f,
                       is_close_to( 0.f, 0.001f ) || is_close_to_rel( 0.f, 0.1f ) );
     }
     SECTION( "Validation" ) {
         REQUIRE_THAT( is_close_to( 1.f, 0.f ).match(0), !throws() );
         REQUIRE_THAT( is_close_to( 1.f, -1.f ).match(0), throws<std::domain_error>() );

         REQUIRE_THAT( is_within_ulp( 1.f, 0 ).match(0), !throws() );
         REQUIRE_THAT( is_within_ulp( 1.f, static_cast<uint64_t>( -1 ) ).match(0),
                            throws<std::domain_error>() );

         REQUIRE_THAT( is_close_to_rel( 1.f, 0.f ).match(0), !throws() );
         REQUIRE_THAT( is_close_to_rel( 1.f, -0.2f ).match(0), throws<std::domain_error>() );
         REQUIRE_THAT( is_close_to_rel( 1.f, 1.f ).match(0), throws<std::domain_error>() );
     }
     SECTION( "IsNaN" ) {
         REQUIRE_THAT( 1., !is_nan() );
         REQUIRE_THAT( NAN, is_nan() );
     }
}

TEST_CASE( "Floating point matchers: double", "[matchers][floating-point]" ) {
    SECTION( "Relative" ) {
        REQUIRE_THAT( 10., is_close_to_rel( 11.1, 0.1 ) );
        REQUIRE_THAT( 10., !is_close_to_rel( 11.2, 0.1 ) );
        REQUIRE_THAT( 1., !is_close_to_rel( 0., 0.99 ) );
        REQUIRE_THAT( -0., is_close_to_rel( 0. ) );
        SECTION( "Some subnormal values" ) {
            auto v1 = std::numeric_limits<double>::min();
            auto v2 = v1;
            for ( int i = 0; i < 5; ++i ) {
                v2 = std::nextafter( v1, 0 );
            }
            REQUIRE_THAT( v1, is_close_to_rel( v2 ) );
        }
    }
    SECTION( "Margin" ) {
        REQUIRE_THAT( 1., is_close_to( 1., 0 ) );
        REQUIRE_THAT( 0., is_close_to( 1., 1 ) );

        REQUIRE_THAT( 0., !is_close_to( 1., 0.99 ) );
        REQUIRE_THAT( 0., !is_close_to( 1., 0.99 ) );

        REQUIRE_THAT( 11., !is_close_to( 10., 0.5 ) );
        REQUIRE_THAT( 10., !is_close_to( 11., 0.5 ) );
        REQUIRE_THAT( -10., is_close_to( -10., 0.5 ) );
        REQUIRE_THAT( -10., is_close_to( -9.6, 0.5 ) );
    }
    SECTION( "ULPs" ) {
        REQUIRE_THAT( 1., is_within_ulp( 1., 0 ) );

        REQUIRE_THAT( nextafter( 1., 2. ), is_within_ulp( 1., 1 ) );
        REQUIRE_THAT( 0., is_within_ulp( nextafter( 0., 1. ), 1 ) );
        REQUIRE_THAT( 1., is_within_ulp( nextafter( 1., 0. ), 1 ) );
        REQUIRE_THAT( 1., !is_within_ulp( nextafter( 1., 2. ), 0 ) );

        REQUIRE_THAT( 1., is_within_ulp( 1., 0 ) );
        REQUIRE_THAT( -0., is_within_ulp( 0., 0 ) );
    }
    SECTION( "Composed" ) {
        REQUIRE_THAT( 1., is_close_to( 1., 0.5 ) || is_within_ulp( 2., 1 ) );
        REQUIRE_THAT( 1., is_close_to( 2., 0.5 ) || is_within_ulp( 1., 0 ) );
        REQUIRE_THAT( 0.0001, is_close_to( 0., 0.001 ) || is_close_to_rel( 0., 0.1 ) );
    }
    SECTION( "Validation" ) {
        REQUIRE_THAT( is_close_to( 1., 0. ).match(0), !throws() );
        REQUIRE_THAT( is_close_to( 1., -1. ).match(0), throws<std::domain_error>() );

        REQUIRE_THAT( is_within_ulp( 1., 0 ).match(0), !throws() );

        REQUIRE_THAT( is_close_to_rel( 1., 0. ).match(0), !throws() );
        REQUIRE_THAT( is_close_to_rel( 1., -0.2 ).match(0), throws<std::domain_error>() );
        REQUIRE_THAT( is_close_to_rel( 1., 1. ).match(0), throws<std::domain_error>() );
    }
    SECTION("IsNaN") {
        REQUIRE_THAT( 1., !is_nan() );
    }
}

TEST_CASE( "Floating point matchers that are problematic in approvals",
           "[approvals][matchers][floating-point]" ) {
    CHECK_THAT( NAN, !is_close_to( NAN, 0 ) );
    CHECK_THAT( NAN, !( is_close_to( NAN, 100 ) || is_within_ulp( NAN, 123 ) ) );
    CHECK_THAT( NAN, !is_within_ulp( NAN, 123 ) );
    CHECK_THAT( INFINITY, is_close_to_rel( INFINITY ) );
    CHECK_THAT( -INFINITY, !is_close_to_rel( INFINITY ) );
    CHECK_THAT( 1., !is_close_to_rel( INFINITY ) );
    CHECK_THAT( INFINITY, !is_close_to_rel( 1. ) );
    CHECK_THAT( NAN, !is_close_to_rel( NAN ) );
    CHECK_THAT( 1., !is_close_to_rel( NAN ) );
    CHECK_THAT( NAN, !is_close_to_rel( 1. ) );
    CHECK_THAT( NAN, is_nan() );
    CHECK_THAT( static_cast<double>(NAN), is_nan() );
}

TEST_CASE( "Arbitrary predicate matcher", "[matchers][generic]" ) {
    SECTION( "Function pointer" ) {
        REQUIRE_THAT( 1, matches_predicate( alwaysTrue, "always true" ) );
        REQUIRE_THAT( 1, !matches_predicate( alwaysFalse, "always false" ) );
    }
    SECTION( "Lambdas + different type" ) {
        REQUIRE_THAT( "Hello olleH",
                      matches_predicate(
                          []( std::string const& str ) -> bool {
                              return str.front() == str.back();
                          },
                          "First and last character should be equal" ) );

        REQUIRE_THAT(
            "This wouldn't pass",
            !matches_predicate( []( std::string const& str ) -> bool {
                return str.front() == str.back();
            } ) );
    }
}

TEST_CASE( "Regression test #1", "[matchers][vector]" ) {
    // At some point, UnorderedEqualsMatcher skipped
    // mismatched prefixed before doing the comparison itself
    std::vector<char> actual = { 'a', 'b' };
    std::vector<char> expected = { 'c', 'b' };

    CHECK_THAT( actual, !equals<InAnyOrder>( expected ) );
}

TEST_CASE( "Predicate matcher can accept const char*",
           "[matchers][compilation]" ) {
    REQUIRE_THAT( "foo", matches_predicate( []( const char* const& ) {
                      return true;
                  } ) );
}

TEST_CASE( "Exceptions matchers", "[matchers][exceptions][!throws]" ) {
    REQUIRE_THAT( throwsDerivedException(),
                throws<DerivedException>().with_message( "DerivedException::what" ) );
    REQUIRE_THAT( throwsDerivedException(),
                throws<DerivedException>().with_message_that( !equals("derivedexception::what") ) );
    REQUIRE_THAT( throwsSpecialException( 2 ),
                throws<SpecialException>().with_message_that( !equals("DerivedException::what" ) ) );
    REQUIRE_THAT( throwsSpecialException( 2 ),
                throws<SpecialException>().with_message( "SpecialException::what" ) );
}

TEST_CASE( "Exception message can be matched", "[matchers][exceptions][!throws]" ) {
    REQUIRE_THAT( throwsDerivedException(),
                    throws<DerivedException>()
                    .with_message_that( starts_with( "Derived" ) ) );
    REQUIRE_THAT( throwsDerivedException(),
                    throws<DerivedException>()
                    .with_message_that( ends_with( "::what" ) ) );
    REQUIRE_THAT( throwsDerivedException(),
                    throws<DerivedException>()
                    .with_message_that( !starts_with( "::what" ) ) );
    REQUIRE_THAT( throwsSpecialException( 2 ),
                    throws<SpecialException>()
                    .with_message_that( starts_with( "Special" ) ) );
}

struct CheckedTestingMatcher {
    bool matchCalled = false;
    bool matchSucceeds = false;

    auto match( int ) -> CatchKit::MatchResult {
        matchCalled = true;
        return matchSucceeds;
    }
    auto describe() const -> CatchKit::MatcherDescription {
        return "CheckedTestingMatcher set to " +
               ( matchSucceeds ? std::string( "succeed" )
                               : std::string( "fail" ) );
    }
};

TEST_CASE( "Composed matchers shortcircuit", "[matchers][composed]" ) {
    // Check that if first returns false, second is not touched
    CheckedTestingMatcher first, second;
    SECTION( "MatchAllOf" ) {
        first.matchSucceeds = false;

        CHECK_THAT( 1, !(first && second) );

        // These two assertions are the important ones
        REQUIRE( first.matchCalled );
        REQUIRE( !second.matchCalled );
    }
    // Check that if first returns true, second is not touched
    SECTION( "MatchAnyOf" ) {
        first.matchSucceeds = true;

        CHECK_THAT( 1, first || second );

        // These two assertions are the important ones
        REQUIRE( first.matchCalled );
        REQUIRE( !second.matchCalled );
    }
}

struct CheckedTestingGenericMatcher {
    bool matchCalled = false;
    bool matchSucceeds = false;

    auto match( int const& ) -> CatchKit::MatchResult {
        matchCalled = true;
        return matchSucceeds;
    }
    auto describe() const -> CatchKit::MatcherDescription {
        return "CheckedTestingGenericMatcher set to " +
               ( matchSucceeds ? std::string( "succeed" )
                               : std::string( "fail" ) );
    }
};

TEST_CASE( "Composed generic matchers shortcircuit",
           "[matchers][composed][generic]" ) {
    // Check that if first returns false, second is not touched
    CheckedTestingGenericMatcher first, second;
    SECTION( "MatchAllOf" ) {
        first.matchSucceeds = false;

        CHECK_THAT( 1, !(first && second) );

        // These two assertions are the important ones
        REQUIRE( first.matchCalled );
        REQUIRE( !second.matchCalled );
    }
    // Check that if first returns true, second is not touched
    SECTION( "MatchAnyOf" ) {
        first.matchSucceeds = true;

        CHECK_THAT( 1, first || second );

        // These two assertions are the important ones
        REQUIRE( first.matchCalled );
        REQUIRE( !second.matchCalled );
    }
}

template <typename Range>
struct EqualsRangeMatcher {

    explicit EqualsRangeMatcher( Range const& range ): m_range{ range } {}

    [[nodiscard]] auto match( auto const& other ) const -> CatchKit::MatchResult {
        using std::begin;
        using std::end;

        return std::equal( begin( m_range ), end( m_range ), begin( other ), end( other ) );
    }

    [[nodiscard]] auto describe() const -> CatchKit::MatcherDescription {
        return std::format("EqualsRange {}", m_range);
    }

private:
    Range const& m_range;
};

template <typename Range>
auto EqualsRange( const Range& range ) -> EqualsRangeMatcher<Range> {
    return EqualsRangeMatcher<Range>{ range };
}

TEST_CASE( "Combining templated matchers", "[matchers][templated]" ) {
    std::array<int, 3> container{ { 1, 2, 3 } };

    std::array<int, 3> a{ { 1, 2, 3 } };
    std::vector<int> b{ 0, 1, 2 };
    std::list<int> c{ 4, 5, 6 };

    REQUIRE_THAT( container,
                  EqualsRange( a ) || EqualsRange( b ) || EqualsRange( c ) );
}


TEST_CASE( "Combining templated and concrete matchers",
           "[matchers][templated]" ) {
    std::vector<int> vec{ 1, 3, 5 };

    std::array<int, 3> a{ { 5, 3, 1 } };

    REQUIRE_THAT( vec,
                  matches_predicate(
                      []( auto const& v ) {
                          return std::all_of(
                              v.begin(), v.end(), []( int elem ) {
                                  return elem % 2 == 1;
                              } );
                      },
                      "All elements are odd" ) &&
                      !EqualsRange( a ) );

    const std::string str = "foobar";
    const std::array<char, 6> arr{ { 'f', 'o', 'o', 'b', 'a', 'r' } };
    const std::array<char, 6> bad_arr{ { 'o', 'o', 'f', 'b', 'a', 'r' } };

    REQUIRE_THAT( str, starts_with( "foo" ) && EqualsRange( arr ) && ends_with( "bar" ) );
    REQUIRE_THAT( str, starts_with( "foo" ) && !EqualsRange( bad_arr ) && ends_with( "bar" ) );

    REQUIRE_THAT( str, EqualsRange( arr ) && starts_with( "foo" ) && ends_with( "bar" ) );
    REQUIRE_THAT( str, !EqualsRange( bad_arr ) && starts_with( "foo" ) && ends_with( "bar" ) );

    REQUIRE_THAT( str, EqualsRange( bad_arr ) || ( starts_with( "foo" ) && ends_with( "bar" ) ) );
    REQUIRE_THAT( str, ( starts_with( "foo" ) && ends_with( "bar" ) ) || EqualsRange( bad_arr ) );
}


struct EvilAddressOfOperatorUsed : std::exception {
    const char* what() const noexcept override {
        return "overloaded address-of operator of matcher was used instead of "
               "std::addressof";
    }
};

struct EvilCommaOperatorUsed : std::exception {
    const char* what() const noexcept override {
        return "overloaded comma operator of matcher was used";
    }
};

struct EvilMatcher {
    auto describe() const -> CatchKit::MatcherDescription { return "equals: 45"; }

    auto match( int i ) const -> CatchKit::MatchResult { return i == 45; }

    EvilMatcher const* operator&() const { throw EvilAddressOfOperatorUsed(); }

    int operator,( EvilMatcher const& ) const { throw EvilCommaOperatorUsed(); }
};

TEST_CASE( "Overloaded comma or address-of operators are not used",
           "[matchers][templated]" ) {
    REQUIRE_THAT( ( EvilMatcher(), EvilMatcher() ),
                       throws<EvilCommaOperatorUsed>() );

    REQUIRE_THAT( &EvilMatcher(), throws<EvilAddressOfOperatorUsed>() );

    REQUIRE_THAT( EvilMatcher() || ( EvilMatcher() && !EvilMatcher()), !throws() );
    REQUIRE_THAT( ( EvilMatcher() && EvilMatcher() ) || !EvilMatcher(), !throws() );
}

struct ImmovableMatcher {
    ImmovableMatcher() = default;
    ImmovableMatcher( ImmovableMatcher const& ) = delete;
    ImmovableMatcher( ImmovableMatcher&& ) = delete;
    ImmovableMatcher& operator=( ImmovableMatcher const& ) = delete;
    ImmovableMatcher& operator=( ImmovableMatcher&& ) = delete;

    CatchKit::MatcherDescription describe() const { return "always false"; }

    template <typename T> CatchKit::MatchResult match( T&& ) const { return false; }
};

struct MatcherWasMovedOrCopied : std::exception {
    const char* what() const noexcept override {
        return "attempted to copy or move a matcher";
    }
};

struct ThrowOnCopyOrMoveMatcher {
    ThrowOnCopyOrMoveMatcher() = default;

    [[noreturn]] ThrowOnCopyOrMoveMatcher( ThrowOnCopyOrMoveMatcher const& other ) {
        throw MatcherWasMovedOrCopied();
    }
    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    [[noreturn]] ThrowOnCopyOrMoveMatcher( ThrowOnCopyOrMoveMatcher&& other ) {
        throw MatcherWasMovedOrCopied();
    }
    ThrowOnCopyOrMoveMatcher& operator=( ThrowOnCopyOrMoveMatcher const& ) {
        throw MatcherWasMovedOrCopied();
    }
    // NOLINTNEXTLINE(performance-noexcept-move-constructor)
    ThrowOnCopyOrMoveMatcher& operator=( ThrowOnCopyOrMoveMatcher&& ) {
        throw MatcherWasMovedOrCopied();
    }

    CatchKit::MatcherDescription describe() const { return "always false"; }

    template <typename T> CatchKit::MatchResult match( T&& ) const { return false; }
};

TEST_CASE( "Matchers are not moved or copied",
           "[matchers][templated][approvals]" ) {
    REQUIRE_THAT(
        ( ThrowOnCopyOrMoveMatcher() && ThrowOnCopyOrMoveMatcher() ) ||
        !ThrowOnCopyOrMoveMatcher(),
        !throws() );
}

TEST_CASE( "Immovable matchers can be used",
           "[matchers][templated][approvals]" ) {
    REQUIRE_THAT( 123,
                  ( ImmovableMatcher() && ImmovableMatcher() ) ||
                      !ImmovableMatcher() );
}

struct ReferencingMatcher {
    CatchKit::MatcherDescription describe() const { return "takes reference"; }
    CatchKit::MatchResult match( int const& i ) const { return i == 22; }
};

TEST_CASE( "Matchers can take references",
           "[matchers][templated][approvals]" ) {
    REQUIRE_THAT( 22, ReferencingMatcher{} );
}

// #ifdef __clang__
// #    pragma clang diagnostic pop
// #endif
//
// TEMPLATE_TEST_CASE(
//     "#2152 - ULP checks between differently signed values were wrong",
//     "[matchers][floating-point][ulp]",
//     float,
//     double ) {
//     using Catch::Matchers::WithinULP;
//
//     static constexpr TestType smallest_non_zero =
//         std::numeric_limits<TestType>::denorm_min();
//
//     CHECK_THAT( smallest_non_zero, WithinULP( -smallest_non_zero, 2 ) );
//     CHECK_THAT( smallest_non_zero, !WithinULP( -smallest_non_zero, 1 ) );
// }
