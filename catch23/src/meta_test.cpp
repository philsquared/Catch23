//
// Created by Phil Nash on 24/07/2025.
//

#include "catch23/meta_test.h"
#include "catch23/runner.h"
#include "catch23/test_registry.h"

#include <algorithm>

namespace CatchKit {

    auto MetaTestResults::failures() const -> int {
        return std::ranges::fold_left( all_results, 0, [](int acc, auto const& result){ return acc + (result.failed() ? 1 : 0); });
    }
    auto MetaTestResults::expected_failures() const -> int {
        return std::ranges::fold_left( all_results, 0, [](int acc, auto const& result){ return acc + (result.failed_expectedly() ? 1 : 0); });
    }

    MetaTestRunner::MetaTestRunner( std::string name, std::source_location location )
    :   name(std::move(name)),
        location(location)
    {}

    auto MetaTestRunner::run( Detail::Test const& test ) && -> MetaTestResults {
        TestRunner runner( reporter, Config() );
        runner.run_test( test );
        return MetaTestResults{ std::move(reporter.results) };
    }

    auto MetaTestRunner::run_test_by_name( std::string const& name_to_find ) && -> MetaTestResults {
        auto const* test = get_test_registry().find_test_by_name( name_to_find );
        if( test == nullptr )
            throw std::domain_error( "No such test: " + name_to_find );
        return std::move(*this).run(*test);
    }

} // namespace CatchKit