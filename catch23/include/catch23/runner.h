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

    void run_test( Test const& test, TestResultHandler& test_handler );

    void run_tests( range_of<Test> auto const& tests, Reporter& reporter ) {
        TestResultHandler test_handler( reporter );

        for( auto&& test : tests) {
            if( !test.test_info.is_manual() )
                run_test(test, test_handler);
        }
    }

} // namespace CatchKit::Detail

#endif // CATCH23_RUNNER_H
