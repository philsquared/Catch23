//
// Created by Phil Nash on 22/07/2025.
//

#include "../include/catch23/console_reporter.h"
#include "../include/catch23/colour.h"
#include "../include/catch23/test_info.h"

#include <print>

#include "catchkit/assertion_context.h"

namespace CatchKit {

    constexpr int console_width = 80; // !TBD: detect/ make configurable

    auto make_ratio( int number, int total ) {
        const auto ratio = total > 0 ? console_width * number / total : 0;
        return (ratio == 0 && number > 0) ? 1 : ratio;
    }

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

            {
                TextColour _(Colours::Red);
                std::print( "{}", std::string( failed_segments, '=' ) );
            }
            {
                TextColour _(Colours::Green);
                std::print( "{}", std::string( passed_segments, '=' ) );
            }
            std::println();
        }
    }

    void ConsoleReporter::lazy_print_test_header() {
        if( lazy_test_info ) {
            std::println("-------------------------------------------------------------------------------");
            {
                TextColour _(Colours::BoldNormal);
                std::println("TEST: {}", lazy_test_info->name);
            }
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

    void ConsoleReporter::on_test_run_end() {
        print_totals_divider(test_totals);

        if ( test_totals.total() == 0 ) {
            TextColour _( Colours::BoldYellow );
            std::println("No tests ran");
            return;
        }

        if ( assertion_totals.total() > 0 && test_totals.all_passed() ) {
            TextColour _( Colours::BoldGreen );
            std::println("All tests passed ({} assertion(s) in {} tests)",
                assertion_totals.total(), test_totals.total() );
            return;
        }

        auto print_summary = [](Counters const& counts, std::string const& label) {

            std::print("{}: {}", label, counts.total() );
            { TextColour _( Colours::BoldGrey ); std::print(" | "); }
            if( counts.passed() > 0) {
                { TextColour _( Colours::Green ); std::print("{} passed", counts.passed() ); }
                if( counts.failed > 0 )
                { TextColour _( Colours::BoldGrey ); std::print(" | "); }
            }
            if( counts.failed > 0) {
                { TextColour _( Colours::Red ); std::print("{} failed", counts.failed ); }
            }
            std::println();
        };
        print_summary(test_totals, "test cases");
        print_summary(assertion_totals, "assertions");
    }

} // namespace CatchKit