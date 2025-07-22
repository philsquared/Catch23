//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_RUNNER_H
#define CATCH23_RUNNER_H

#include "catch23_test.h"
#include "catch23_reporter.h"
#include "catch23_test_result_handler.h"

namespace CatchKit::Detail {

    template<typename R, typename T>
    concept range_of = std::ranges::range<R> &&
                       std::same_as<std::ranges::range_value_t<R>, T>;

    void run_tests( range_of<Test> auto const& tests, Reporter& reporter ) {
        TestResultHandler test_handler( reporter );
        bool should_report_success = false;

        for( auto&& test : tests) {

            // !TBD: this skips hidden tests until we do proper tag parsing
            if( test.test_info.tags.find("[.") != std::string::npos )
                continue;

            // ReentryNodes nodes;
            Checker check{
                .result_handler=test_handler,
                .result_disposition=ResultDisposition::Continue,
                .should_report_success=should_report_success };
            Checker require{
                .result_handler=test_handler,
                .result_disposition=ResultDisposition::Abort,
                .should_report_success=should_report_success };

            reporter.on_test_start(test.test_info);
            try {
                test.test_fun(check, require);
            }
            catch( TestCancelled ) {
                // std::println("  *** aborted"); // !TBD
            }
            catch( ... ) {
                // We need a new context because the old one had string_views to outdated data
                // - we want to preserve the last known source location, though
                AssertionContext context{
                    .macro_name = "",
                    .original_expression = "* unknown line after the reported location *",
                    .message = {},
                    .location = test_handler.get_current_context().location };
                test_handler.on_assertion_start( CatchKit::ResultDisposition::Continue, std::move(context) );
                test_handler.on_assertion_result( CatchKit::ResultType::UnexpectedException, {}, CatchKit::Detail::get_exception_message(std::current_exception()) );
            }
            reporter.on_test_end(test.test_info); // !TBD: report number of assertions passed/ failed and overall status
        }
    }

} // namespace CatchKit::Detail

#endif // CATCH23_RUNNER_H
