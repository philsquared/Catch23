//
// Created by Phil Nash on 24/07/2025.
//

#include "catch23/meta_test.h"

#include <algorithm>

#include "catch23/runner.h"

namespace CatchKit {

    auto MetaTestResults::failures() const -> int {
        return std::ranges::fold_left( all_results, 0, [](int acc, auto const& result){ return acc += result.failed(); });
    }

    MetaTestRunner::MetaTestRunner( std::string name, std::source_location location )
    :   handler( reporter ),
        name(std::move(name)),
        location(location)
    {}

    auto MetaTestRunner::run( Detail::Test const& test ) && -> MetaTestResults {
        run_test(test, handler);
        return MetaTestResults{ std::move(reporter.results) };
    }

    auto MetaTestRunner::run_test_by_name( std::string const& name ) && -> MetaTestResults {
        auto const* test = Detail::find_test_by_name( name );
        if( test == nullptr )
            throw std::domain_error( "No such test: " + name );
        return std::move(*this).run(*test);
    }

} // namespace CatchKit