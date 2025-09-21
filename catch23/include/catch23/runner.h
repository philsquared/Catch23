//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_RUNNER_H
#define CATCH23_RUNNER_H

#include "test.h"
#include "reporter.h"
#include "test_result_handler.h"

namespace CatchKit::Detail {

    template<typename R, typename T>
    concept range_of = std::ranges::range<R> &&
                       std::same_as<std::ranges::range_value_t<R>, T>;

    class TestRunner {
        TestResultHandler result_handler;
    public:
        explicit TestRunner( Reporter& reporter ) : result_handler(reporter) {}

        void run_test( Test const& test );

        void run_tests( range_of<Test> auto const& tests ) {
            result_handler.get_reporter().on_test_run_start();

            // !TBD Apply filters
            for( auto&& test : tests) {
                if( !test.test_info.is_manual() ) // for now, until we pre-filter
                    run_test( test );
            }

            result_handler.get_reporter().on_test_run_end();
        }
    };

} // namespace CatchKit::Detail

namespace CatchKit {
    using Detail::TestRunner;
}

#endif // CATCH23_RUNNER_H
