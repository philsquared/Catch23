//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_CONSOLE_REPORTER_H
#define CATCH23_CONSOLE_REPORTER_H

#include "catch23_reporter.h"

namespace CatchKit {

    struct ConsoleReporter : Reporter {
        std::optional<TestInfo> lazy_test_info;
        void lazy_print_test_header();
        void on_test_start( TestInfo const& test_info ) override;
        void on_test_end( TestInfo const& test_info ) override;

        void on_assertion_start( AssertionContext const& context ) override;
        void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) override;
    };

} // namespace CatchKit

#endif //CATCH23_CONSOLE_REPORTER_H
