//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/console_reporter.h"

#include <cassert>

#include "catch23/print.h"
#include "catch23/test_info.h"

#include "catchkit/assertion_context.h"
#include "catchkit/expression_info.h"

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
        if( !printed_header && current_test_info ) {
            std::println("-------------------------------------------------------------------------------");
            println(ColourIntent::Headers, "TEST: {}", current_test_info->name);
            std::println("{}:{}",
                    current_test_info->location.file_name(),
                    current_test_info->location.line());
            std::println("...............................................................................\n");
            printed_header = false;
        }
    }
    void ConsoleReporter::on_test_start( TestInfo const& test_info ) {
        current_test_info = &test_info;
        printed_header = false;
    }
    void ConsoleReporter::on_test_end( TestInfo const&, Counters const& assertions ) {
        // !TBD: should probably move this bit to on_assertion_end and use +=
        if( assertions.failed == 0 )
            test_totals.passed_explicitly++;
        else
            test_totals.failed++;
        assertion_totals += assertions;
        current_test_info = nullptr;
    }

    void ConsoleReporter::on_assertion_start( AssertionContext const& ) {}

    void ConsoleReporter::on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) {
        assert(current_test_info);
        lazy_print_test_header();
        std::print( "{}:{}:{}: ",
                context.location.file_name(),
                context.location.line(),
                context.location.column());
        if( assertion_info.passed() ) {
            if( current_test_info->should_fail() )
                println( ColourIntent::Success, "👍 PASSED" );
            else
                println( ColourIntent::Error, "🤦 PASSED - but expected failure" );
        }
        else {
            if( current_test_info->should_fail() )
                println( ColourIntent::Success, "👍 FAILED as expected" );
            else if( current_test_info->may_fail() )
                println( ColourIntent::Warning, "🫡 FAILED, but ok" );
            else
                println( ColourIntent::Error, "❌ FAILED" );
        }

        if( !context.original_expression.empty() )
            std::println( "for expression:" );

        if( context.macro_name.empty() ) {
            if( !context.original_expression.empty() )
                println( ColourIntent::OriginalExpression, "  {}", context.original_expression );
        }
        else {
            if( !context.original_expression.empty() )
                println( ColourIntent::OriginalExpression, "  {}( {} )", context.macro_name, context.original_expression );
            else
                println( ColourIntent::OriginalExpression, "  for {}", context.macro_name );
        }

        if( auto const& expr_info = assertion_info.expression_info;
                !std::holds_alternative<std::monostate>( expr_info ) ) {
            if( auto except_expr = std::get_if<ExceptionExpressionInfo>( &expr_info ) ) {
                switch( except_expr->type ) {
                case ExceptionExpressionInfo::Type::Unexpected:
                    std::println("due to an unexpected exception");
                    break;
                case ExceptionExpressionInfo::Type::Missing:
                    std::println("due to a missing exception");
                    break;
                default:
                    assert( false );
                }
            }
            std::println("with expansion:");
            print( ColourIntent::ReconstructedExpression, "  {}", expr_info );

            if( auto const* match_expr = std::get_if<MatchExpressionInfo>(&expr_info) ) {
                if( assertion_info.passed() )
                    std::print(" - matched");
                else
                    std::print(" - failed to match");

                if( !match_expr->sub_expressions.empty() ) {
                    std::println( " because:");
                    for(auto const& sub_expr : match_expr->sub_expressions) {
                        if( sub_expr.result )
                            std::print("    ✅ ");
                        else
                            std::print("    ❌ ");
                        print( ColourIntent::ReconstructedExpression, "{} ", sub_expr.description );
                        if( sub_expr.result )
                            std::println("matched");
                        else
                            std::println("failed to match");
                    }
                }
            }
            std::println();
        }
        if (!assertion_info.message.empty()) {
            std::println("with message:");
            println(ColourIntent::SecondaryText, "  {}", assertion_info.message);
        }
    }

    void ConsoleReporter::on_shrink_start() {
        std::println("Attempting to find a simpler counterexample by \"shrinking\"...");
    }
    void ConsoleReporter::on_no_shrink_found( int shrinks ) {
        std::println("\nNo simpler counterexample found after {} shrinks", shrinks);
    }
    void ConsoleReporter::on_shrink_result( ResultType result, int shrinks_so_far ) {
        const int shrink_print_width = 37;
        if( shrinks_so_far < shrink_print_width ) {
            if( result == ResultType::Passed )
                std::print("✅");
            else
                std::print("❌");
        }
        else if( shrinks_so_far == shrink_print_width )
            std::print("... ");
    }
    void ConsoleReporter::on_shrink_found( std::vector<std::string> const& values, int shrinks ) {
        std::println("\nFalsifiable after {} shrinks:", shrinks);
        if( values.size() > 1 ) {
            int i = 0;
            for (auto const& value : values) {
                std::println("  value {}: {}", ++i, value);
            }
            std::println("Final run with these values:");
        }
        else {
            std::println("  value: {}", values[0]);
            std::println("Final run with this value:");
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