//
// Created by Phil Nash on 24/07/2025.
//

#include "../include/catch23/local_test.h"
#include "../include/catch23/runner.h"

namespace CatchKit {
    MetaTestRunner::MetaTestRunner( std::string name, std::source_location location )
    :   handler( reporter ),
        name(std::move(name)),
        location(location)
    {}

    auto MetaTestRunner::run( Detail::Test const& test ) && -> std::vector<FullAssertionInfo> {
        run_test(test, handler);
        return std::move(reporter.results);
    }

} // namespace CatchKit