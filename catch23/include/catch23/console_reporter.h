//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_CONSOLE_REPORTER_H
#define CATCH23_CONSOLE_REPORTER_H

#include "reporter.h"

namespace CatchKit {

    class ConsoleReporter : public Reporter {
        TestInfo const* current_test_info = nullptr;
        bool printed_header = false;
        ReportOn what_to_report_on;
        void lazy_print_test_header();

        Counters test_totals;
        Counters assertion_totals;

    public:
        explicit ConsoleReporter( ReportOn what_to_report_on = ReportOn::FailuresOnly )
        : what_to_report_on( what_to_report_on )
        {}

        auto report_on_what() const -> ReportOn override {
            return what_to_report_on;
        }

        void on_test_start( TestInfo const& test_info ) override;
        void on_test_end( TestInfo const& test_info, Counters const& assertions ) override;

        void on_assertion_start( AssertionContext const& context ) override;
        void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) override;

        void on_shrink_start() override;
        void on_shrink_found( std::vector<std::string> const& values, int shrinks ) override;
        void on_no_shrink_found( int shrinks ) override;
        void on_shrink_result( ResultType result, int shrinks_so_far ) override;

        void on_test_run_start() override;
        void on_test_run_end() override;
    };

} // namespace CatchKit

#endif // CATCH23_CONSOLE_REPORTER_H
