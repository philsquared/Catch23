#include "catch23/catch23_console_reporter.h"
#include "catch23/catch23_test.h"
#include "catch23/catch23_test_result_handler.h"

#include "catchkit/catchkit_matchers.h"

#include <print>

TEST("test a") {
    CHECK( 1 == 2 );
    throw std::domain_error("I didn't expect that!");
}

TEST("test b") {
    REQUIRE_THAT("one", equals("one"));
}

namespace CatchKit {


}

void run_tests() {
    CatchKit::TestResultHandler test_handler( std::make_unique<CatchKit::ConsoleReporter>() );
    CatchKit::Checker check{ test_handler, CatchKit::ResultDisposition::Continue };
    CatchKit::Checker require{ test_handler, CatchKit::ResultDisposition::Abort };

    for( auto&& test : CatchKit::Detail::get_all_tests()) {
        auto& reporter = test_handler.get_reporter();
        reporter.on_test_start(test.test_info);
        try {
            test.test_fun(check, require);
        }
        catch( CatchKit::Detail::TestCancelled ) {
            std::println("  *** aborted"); // !TBD
        }
        catch( ... )
        {
            // We need a new context because the old one had string_views to outdated data
            // - we want to preserve the last known source location, though
            CatchKit::AssertionContext context{
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
int main() {
    run_tests();
}