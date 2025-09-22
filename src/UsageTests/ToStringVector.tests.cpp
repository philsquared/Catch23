#include <catch23/catch2_compat.h>
#include <vector>
#include <array>

// vector
TEST_CASE( "vector<int> -> toString", "[toString][vector]" )
{
    std::vector<int> vv;
    CHECK( ::CatchKit::stringify(vv) == "[]" );
    vv.push_back( 42 );
    CHECK( ::CatchKit::stringify(vv) == "[42]" );
    vv.push_back( 250 );
    CHECK( ::CatchKit::stringify(vv) == "[42, 250]" );
}

TEST_CASE( "vector<string> -> toString", "[toString][vector]" )
{
    std::vector<std::string> vv;
    CHECK( ::CatchKit::stringify(vv) == "[]" );
    vv.emplace_back( "hello" );
    CHECK( ::CatchKit::stringify(vv) == "[\"hello\"]" );
    vv.emplace_back( "world" );
    CHECK( ::CatchKit::stringify(vv) == "[\"hello\", \"world\"]" );
}

namespace {
    /* Minimal Allocator */
    template<typename T>
    struct minimal_allocator {
        using value_type = T;
        using size_type = std::size_t;

        minimal_allocator() = default;
        template <typename U>
        minimal_allocator(const minimal_allocator<U>&) {}


        T *allocate( size_type n ) {
            return static_cast<T *>( ::operator new( n * sizeof(T) ) );
        }
        void deallocate( T *p, size_type /*n*/ ) {
            ::operator delete( static_cast<void *>(p) );
        }
        template<typename U>
        bool operator==( const minimal_allocator<U>& ) const { return true; }
        template<typename U>
        bool operator!=( const minimal_allocator<U>& ) const { return false; }
    };
}

TEST_CASE( "vector<int,allocator> -> toString", "[toString][vector,allocator]" ) {
    std::vector<int,minimal_allocator<int> > vv;
    CHECK( ::CatchKit::stringify(vv) == "[]" );
    vv.push_back( 42 );
    CHECK( ::CatchKit::stringify(vv) == "[42]" );
    vv.push_back( 250 );
    CHECK( ::CatchKit::stringify(vv) == "[42, 250]" );
}

TEST_CASE( "vec<vec<string,alloc>> -> toString", "[toString][vector,allocator]" ) {
    using inner = std::vector<std::string, minimal_allocator<std::string>>;
    using vector = std::vector<inner>;
    vector v;
    CHECK( ::CatchKit::stringify(v) == "[]" );
    v.push_back( inner { "hello" } );
    v.push_back( inner { "world" } );
    CHECK( ::CatchKit::stringify(v) == "[[\"hello\"], [\"world\"]]" );
}

// Based on PR by mat-so: https://github.com/catchorg/Catch2/pull/606/files#diff-43562f40f8c6dcfe2c54557316e0f852
TEST_CASE( "vector<bool> -> toString", "[toString][containers][vector]" ) {
    std::vector<bool> bools;
    REQUIRE( ::CatchKit::stringify(bools) == "[]");
    bools.push_back(true);
    REQUIRE( ::CatchKit::stringify(bools) == "[true]");
    bools.push_back(false);
    REQUIRE( ::CatchKit::stringify(bools) == "[true, false]");
}

TEST_CASE( "array<int, N> -> toString", "[toString][containers][array]" ) {
    std::array<int, 0> empty;
    REQUIRE( CatchKit::stringify( empty ) == "[]" );
    std::array<int, 1> oneValue = {{ 42 }};
    REQUIRE( CatchKit::stringify( oneValue ) == "[42]" );
    std::array<int, 2> twoValues = {{ 42, 25 }};
    REQUIRE( CatchKit::stringify( twoValues ) == "[42, 25]" );
}

