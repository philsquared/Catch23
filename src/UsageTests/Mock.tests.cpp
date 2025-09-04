//
// Created by Phil Nash on 15/08/2025.
//

#include "catch23/mocks.h"
#include "catch23/test.h"
#include "catch23/meta_test.h"
#include "catchkit/matchers.h"


struct NotDefaultConstructible {
    int i;
    explicit NotDefaultConstructible(int i) : i(i) {}
};

namespace TestNS {
    struct Interface {
        virtual ~Interface() = default;
        virtual void foo(int i) = 0;
        virtual int bar() const = 0;
    };
}

// !TBD: One limitation for now is that we must declare our mocks at global scope _and_ use an unqualified name
// for the type we are mocking (which requires us to bring it into the global namespace).
// If we can't come up with a better way to do it we could allow a separate macro arg for the namespace/ scope?
using TestNS::Interface;
MOCK(Interface) {
    MOCK_METHOD(      , void, foo, int);
    MOCK_METHOD( const, int, bar );
}

struct InterfaceWithNoDefaultConstructibleReturn {
    virtual ~InterfaceWithNoDefaultConstructibleReturn() = default;
    virtual NotDefaultConstructible foobar() const = 0;
};
MOCK(InterfaceWithNoDefaultConstructibleReturn) {
    MOCK_METHOD( const, NotDefaultConstructible, foobar );
}

int use_mock(Interface& i) {
    i.foo(42);
    i.foo(42);
    return i.bar();
}

TEST("No expectations set") {
    auto results = LOCAL_TEST() {
        using namespace CatchKit::Mocks;
        MockFor<Interface> stubbed;
        use_mock(stubbed);
    };
    REQUIRE( results.failed() );
    CHECK_THAT( results.message(), contains("forget to call set"));
}

TEST("Specific expectation not set") {
    auto results = LOCAL_TEST() {
        using namespace CatchKit::Mocks;
        MockFor<Interface> stubbed;
        auto expectations = expect( stubbed.foo() ).set(); // Only foo() set
        use_mock(stubbed);
    };
    REQUIRE( results.failed() );
    CHECK_THAT( results.message(), contains("Interface::bar") && contains("exhausted"));
}

TEST("Stubs need minimal setup") {
    using namespace CatchKit::Mocks;
    MockFor<Interface> stubbed;
    auto expectations = expect( stubbed._() ).set(); // This says any calls are ok
    CHECK( use_mock(stubbed) == 0 );
}

TEST("Simple mock that works") {
    using namespace CatchKit::Mocks;
    MockFor<Interface> m;
    auto expectations = expect(
        // m.foo().called(once),
        m.foo(42).called(at_most( 2_times )),
        m.bar().called(once).returns(7)
    ).set();
    int i = use_mock(m); // a call that uses the mock
    CHECK(i == 7);
}

// WIP more complex cases
TEST("Mocks", "[/.]") {
    using namespace CatchKit::Mocks;
    MockFor<Interface> m;
    // auto stub_m = expect(
    //     m._(),
    //     m.foobar().returns(NotDefaultConstructible(1)) );
    // stub_m.set();
    // auto ndc_expectations = expect(
    //     m.foobar().returns(NotDefaultConstructible(1))
    // );
    auto expectations = expect(
        // m._().called(maybe_once),
        // std::move(ndc_expectations), // !TBD: We should make these copyable
        // m.foo().called(never),
        // m.foo().called(once),
        m.foo(42).called(2_times),
        expect<sequenced::in_order>(
            m.bar().called(once).returns(7),
            m.bar().called(once), // return default
            m.bar().called(never) // allow no more calls
        )
    ).set();
    int i = use_mock(m); // a call that uses the mock
    CHECK(i == 7);
    // CHECK_THAT(expectations, completed()); // Optional, but allows us to control, e.g. exceptions - also not implemented, yet
}

using CatchKit::Reporter;
MOCK(Reporter) {
    MOCK_METHOD(const, CatchKit::ReportOn, report_on_what );
    MOCK_METHOD(, void, on_test_start, CatchKit::TestInfo const& );
    MOCK_METHOD(, void, on_test_end, CatchKit::TestInfo const&, CatchKit::Counters const& );
    MOCK_METHOD(, void, on_assertion_start, CatchKit::AssertionContext const& );
    MOCK_METHOD(, void, on_assertion_end, CatchKit::AssertionContext const&, CatchKit::AssertionInfo const& );
    MOCK_METHOD(, void, on_shrink_start );
    MOCK_METHOD(, void, on_shrink_found, std::vector<std::string> const&, int );
    MOCK_METHOD(, void, on_no_shrink_found, int );
    MOCK_METHOD(, void, on_shrink_result, CatchKit::ResultType, int );
}

TEST("Mock Reporter") {
    using namespace CatchKit::Mocks;
    MockFor<CatchKit::Reporter> m;
    CatchKit::Reporter& r = m;

    // Just a stub
    auto expectations = expect( m._() ).set();
    r.on_shrink_start();
}
