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
    REQUIRE_THAT("one", equals("two"));
}

namespace CatchKit {

    struct ConsoleReporter : Reporter {
        void on_test_start( TestInfo const& test_info ) override {
            std::println("Running test: {}", test_info.name);
        }
        void on_test_end( TestInfo const& test_info ) override {
            std::println("Finished: {}\n", test_info.name);
        }

        void on_assertion_start( AssertionContext const& context ) override {
            std::println("{}:{}:{}: in function '{}'",
                    context.location.file_name(),
                    context.location.line(),
                    context.location.column(),
                    context.location.function_name());
        }
        void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) override {
            std::string_view macro_name = context.macro_name;
            if( macro_name.empty() )
                macro_name = "assertion";
            if( !context.original_expression.empty() )
                std::println("{} {} for expression:\n\t{}",
                        macro_name, assertion_info.passed() ? "passed" : "failed",
                        context.original_expression);
            else
                std::println("{} {}",
                    macro_name, assertion_info.passed() ? "passed" : "failed");

            switch( assertion_info.result ) {
            case ResultType::UnexpectedException:
                std::println("due to an unexpected exception");
                break;
            case ResultType::MissingException:
                std::println("due to a missing exception");
                break;
            default:
                if(assertion_info.expression_info)
                    std::println("with expansion:\n\t{}", *assertion_info.expression_info);
                break;
            }
            if (!assertion_info.message.empty()) {
                std::println("with message:\n\t{}", assertion_info.message);
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
            CatchKit::AssertionContext context{
                .macro_name = "test",
                .original_expression = {},
                .message = {},
                .location = test_handler.get_current_context().location };
            test_handler.on_assertion_start( CatchKit::ResultDisposition::Continue, std::move(context) );
            test_handler.on_assertion_result( CatchKit::ResultType::UnexpectedException, {}, CatchKit::Detail::get_exception_message(std::current_exception()) );
        }
        reporter.on_test_end(test.test_info);
    }

}
int main() {
    run_tests();
}