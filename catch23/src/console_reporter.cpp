//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/console_reporter.h"
#include "catch23/print.h"
#include "catch23/test_info.h"

#include "catchkit/assertion_context.h"

namespace CatchKit {

    constexpr int console_width = 80; // !TBD: detect/ make configurable


    void print_totals_divider(Counters const& totals) {
        if (totals.total() > 0) {
            int passed_segments = console_width * totals.passed() / totals.total();
            int failed_segments = console_width * totals.failed / totals.total();
            if( passed_segments + failed_segments < console_width ) {
                if( passed_segments > failed_segments )
                    failed_segments++;
                else
                    passed_segments++;
            }

            print( ColourIntent::ResultError, "{}", std::string( failed_segments, '=' ) );
            print( ColourIntent::ResultSuccess, "{}", std::string( passed_segments, '=' ) );
            std::println();
        }
    }

    void ConsoleReporter::lazy_print_test_header() {
        if( lazy_test_info ) {
            std::println("-------------------------------------------------------------------------------");
            println(ColourIntent::Headers, "TEST: {}", lazy_test_info->name);
            std::println("{}:{}",
                    lazy_test_info->location.file_name(),
                    lazy_test_info->location.line());
            std::println("...............................................................................\n");
            lazy_test_info.reset();
        }
    }
    void ConsoleReporter::on_test_start( TestInfo const& test_info ) {
        lazy_test_info = test_info;
    }
    void ConsoleReporter::on_test_end( TestInfo const&, Counters const& assertions ) {
        if( assertions.failed == 0 )
            test_totals.passed_explicitly++;
        else
            test_totals.failed++;
        assertion_totals += assertions;
    }

    void ConsoleReporter::on_assertion_start( AssertionContext const& ) {
    }
    void ConsoleReporter::on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) {
        lazy_print_test_header();
        println( assertion_info.passed() ? ColourIntent::Success : ColourIntent::Error,
                "{}:{}:{}: {}",
                context.location.file_name(),
                context.location.line(),
                context.location.column(),
                assertion_info.passed() ? "👍 PASSED" : "❌ FAILED");

        if( !context.original_expression.empty() )
            std::println( "for expression:" );

        if( context.macro_name.empty() ) {
            if( !context.original_expression.empty() )
                println( ColourIntent::OriginalExpression, "\t{}", context.original_expression );
        }
        else {
            if( !context.original_expression.empty() )
                println( ColourIntent::OriginalExpression, "\t{}( {} )", context.macro_name, context.original_expression );
            else
                println( ColourIntent::OriginalExpression, "\tfor {}", context.macro_name );
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
                println(ColourIntent::ReconstructedExpression, "\t{}", *assertion_info.expression_info);
            }
            break;
        }
        if (!assertion_info.message.empty()) {
            std::println("with message:");
            println(ColourIntent::SecondaryText, "\t{}", assertion_info.message);
        }
    }

    void ConsoleReporter::on_test_run_end() {
        print_totals_divider(test_totals);

        if ( test_totals.total() == 0 ) {
            println( ColourIntent::Warning, "No tests ran" );
            return;
        }

        if ( assertion_totals.total() > 0 && test_totals.all_passed() ) {
            println( ColourIntent::ResultSuccess, "All tests passed ({} assertion(s) in {} tests)",
                assertion_totals.total(), test_totals.total() );
            return;
        }

        auto print_summary = [](Counters const& counts, std::string const& label) {

            std::print("{}: {}", label, counts.total() );
            print( Colours::BoldGrey, " | " );
            if( counts.passed() > 0) {
                print( ColourIntent::ResultSuccess, "{} passed", counts.passed() );
                if( counts.failed > 0 )
                    print( Colours::BoldGrey, " | " );
            }
            if( counts.failed > 0) {
                print( ColourIntent::ResultError, "{} failed", counts.failed );
            }
            std::println();
        };
        print_summary(test_totals, "test cases");
        print_summary(assertion_totals, "assertions");
    }

} // namespace CatchKit