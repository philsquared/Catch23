//
// Created by Phil Nash on 02/09/2025.
//
#include "catch23/mocks.h"

#include "catchkit/checker.h"

#include <format>
#include <utility>

namespace CatchKit::Mocks {

    const MatchesAnything _;

    void return_void() {}

    [[noreturn]] static void fail(std::source_location location, std::string const& message) {
        {
            // This is the only part that is dependent on CatchKit/ Catch23
            // We could isolate it more and make it more substitutable for use with other frameworks?
            auto _ = require("", location);
            require.result_handler->on_assertion_result(
                ResultType::MatchFailed,
                {},
                message );
        }
        // The destructor of the asserter should detect that we failed and throw an exception
        // so we should never reach here
        std::unreachable();
    }

    auto MethodBase::get_root_expectation() -> ExpectationsBase* {
        return mock_base->expectation;
    }
    auto MethodBase::get_qualified_name() const -> std::string {
        return std::format("{}::{}", mock_base->name, name);
    }

    void MethodBase::fail(std::string const& message) const {
        Mocks::fail(location, message);
    }

    ExpectationImplBase::ExpectationImplBase( MethodBase* method, std::source_location location )
    :   method( method ),
        location(location)
    {
        is_wildcard = method->is_wildcard;
    }

    void ExpectationImplBase::set(ExpectationsBase* root) {
        method->mock_base->expectation = root;
    }
    void ExpectationImplBase::reset() {
        method->mock_base->expectation = nullptr;
    }

    void ExpectationImplBase::find_expectations_for_method( MockBase* obj_addr, MethodBase* method_addr, std::vector<IExpectation*>& matching ) {
        if( method == method_addr || (is_wildcard && obj_addr == method->mock_base) ) {
            matching.push_back( this );
        }
    }

    void ExpectationImplBase::fail(std::string const& message) const {
        Mocks::fail(location, message);
    }

    auto describe_times(int times) -> std::string {
        if( times == 0 )
            return "zero times";
        if( times == 1 )
            return "once";
        return std::format("{} times", times);
    }

    void ExpectationImplBase::verify_within_max() const {
        if( invocations > cardinality.max ) {
            if( cardinality.is_range() ) {
                fail(std::format("{} should have been called at most {} but has been called {}",
                        method->get_qualified_name(), describe_times(cardinality.max), describe_times(invocations) ));
            }
            else {
                fail(std::format("{} should have been called exactly {} but has been called {}",
                        method->get_qualified_name(), describe_times(cardinality.max), describe_times(invocations) ));
            }
        }
    }

    void ExpectationImplBase::verify() const {
        verify_within_max();
        if( invocations < cardinality.min ) {
            if( cardinality.is_range() ) {
                fail( std::format("{} should have been called at least {} but has only been called {}",
                    method->get_qualified_name(), describe_times(cardinality.min), describe_times(invocations) ) );
            }
            else {
                fail( std::format("{} should have been called exactly {} but has only been called {}",
                    method->get_qualified_name(), describe_times(cardinality.min), describe_times(invocations) ) );
            }
        }
    }

} // namespace CatchKit::Mocks
