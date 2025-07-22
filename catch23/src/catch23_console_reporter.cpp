//
// Created by Phil Nash on 22/07/2025.
//

#include "../include/catch23/catch23_console_reporter.h"
#include "../include/catch23/catch23_colour.h"
#include "../include/catch23/catch23_test_info.h"

#include <print>

#include "catchkit/catchkit_assertion_context.h"

namespace CatchKit {


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
    void ConsoleReporter::on_test_end( TestInfo const& ) {
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

} // namespace CatchKit