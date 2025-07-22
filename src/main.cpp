#include "catch23/catch23_reporter.h"
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

#include "catch23/catch23_colour.h"

namespace CatchKit {

    struct ConsoleReporter : Reporter {
        void on_test_start( TestInfo const& test_info ) override {
            std::println("-------------------------------------------------------------------------------");
            {
                TextColour _(Colours::BoldNormal);
                std::println("TEST: {}", test_info.name);
            }
            std::println("{}:{}",
                    test_info.location.file_name(),
                    test_info.location.line());
            std::println("...............................................................................\n");
        }
        void on_test_end( TestInfo const& test_info ) override {
            std::println("Finished: {}\n", test_info.name);
        }

        void on_assertion_start( AssertionContext const& context ) override {
        }
        void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) override {
            {
                TextColour colour;
                if ( assertion_info.passed() )
                    colour.set(Colours::Green);
                else
                    colour.set(Colours::Red);
                std::println("{}:{}:{}: {}",
                        context.location.file_name(),
                        context.location.line(),
                        context.location.column(),
                        assertion_info.passed() ? "PASSED" : "FAILED");
            }

            if( !context.original_expression.empty() )
                std::println( "for expression:" );

            {
                TextColour colour(Colours::Cyan);
                if( context.macro_name.empty() ) {
                    if( !context.original_expression.empty() )
                        std::println( "\t{}", context.original_expression );
                }
                else {
                    if( !context.original_expression.empty() )
                        std::println( "\t{}( {} )", context.macro_name, context.original_expression );
                    else
                        std::println( "\tfor {}", context.macro_name );
                }
            }

            switch( assertion_info.result ) {
            case ResultType::UnexpectedException:
                std::println("due to an unexpected exception");
                break;
            case ResultType::MissingException:
                std::println("due to a missing exception");
                break;
            default:
                if(assertion_info.expression_info) {
                    std::println("with expansion:");
                    TextColour _(Colours::BoldYellow);
                    std::println("\t{}", *assertion_info.expression_info);
                }
                break;
            }
            if (!assertion_info.message.empty()) {
                std::println("with message:");
                TextColour _(Colours::BoldWhite);
                std::println("\t{}", assertion_info.message);
            }
        }
    };
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