//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/test_result_handler.h"
#include "catchkit/assertion_context.h"

#include "catchkit/variable_capture.h"

#include <cassert>

namespace CatchKit::Detail {

    TestResultHandler::TestResultHandler(Reporter& reporter)
    :   ResultHandler(reporter.report_on_what()),
        reporter(reporter)
    {}

    void TestResultHandler::on_assertion_start( ResultDisposition result_disposition, AssertionContext&& context ) {
        current_context = std::move(context);
        this->result_disposition = result_disposition;
        reporter.on_assertion_start( context );
    }

    void TestResultHandler::on_shrink_start() {
        shrinking_mode = ShrinkingMode::Shrinking;
        reporter.on_shrink_start();
        shrink_count = 0;
    }
    void TestResultHandler::on_shrink_found( std::vector<std::string> const& values ) {
        if( values.empty() ) {
            reporter.on_no_shrink_found( shrink_count );
            shrinking_mode = ShrinkingMode::NotShrunk;
            return;
        }
        shrinking_mode = ShrinkingMode::Shrunk;
        reporter.on_shrink_found( values, shrink_count );
    }

    void TestResultHandler::on_shrink_end() {
        shrinking_mode = ShrinkingMode::Normal;
    }

    void TestResultHandler::on_assertion_result( ResultType result, ExpressionType expression_type, std::optional<ExpressionInfo> const& expression_info, std::string_view message ) {
        last_result = result;
        if( shrinking_mode == ShrinkingMode::Shrinking ) {
            shrink_count++;
            reporter.on_shrink_result(result, shrink_count);
            return;
        }
        if( shrinking_mode == ShrinkingMode::NotShrunk )
            return;

        if( shrinking_mode != ShrinkingMode::Shrunk ) {
            if( result == ResultType::Passed )
                assertions.passed_explicitly++;
            else
                assertions.failed++;
        }

        // !TBD: We should need to check this again
        // - go back to having two on_assertion_result methods - one that takes just a result,
        // the other takes the full, expanded, data (probably no need for optional)
        if( report_on == ReportOn::AllResults || result != ResultType::Passed ) {
            if( !message.empty() ) {
                // Attempt to string out an inline message
                // note: this is quite brittle, so if it seems to have stopped working
                // check that this logic still matches usage
                if( auto pos = current_context.original_expression.rfind(message); pos != std::string::npos ) {
                    assert( pos > 3 && current_context.original_expression[pos-3] == ',' );
                    current_context.original_expression = current_context.original_expression.substr( 0, pos-3 );
                }
            }

            std::string full_message(message);

            // Add in any captured variables
            // !TBD: improve formatting - or should we pass this through in a more fine-grained way to the reporter?
            if( !variable_captures.empty() ) {
                if( !full_message.empty() )
                    full_message += "\nwith";
                full_message += "captured variables:";
                for( auto var : variable_captures ) {
                    full_message += std::format("\n    {} : {} = {}", var->name, var->type, var->get_value() );
                }
            }
            reporter.on_assertion_end(current_context, AssertionInfo{ result, expression_type, expression_info, full_message } );
        }
    }

    void TestResultHandler::on_assertion_end() {
        if( last_result != ResultType::Passed && result_disposition == ResultDisposition::Abort ) {
            throw TestCancelled();
        }
    }

    void TestResultHandler::add_variable_capture( VariableCapture* capture ) {
        variable_captures.push_back(capture);
    }
    void TestResultHandler::remove_variable_capture( VariableCapture* capture ) {
        std::erase(variable_captures, capture);
    }

    auto get_execution_nodes_from_result_handler(ResultHandler& handler) -> ExecutionNodes& {
        assert(dynamic_cast<TestResultHandler*>(&handler) != nullptr);
        auto execution_nodes = static_cast<TestResultHandler&>(handler).get_execution_nodes();
        assert(execution_nodes);
        return *execution_nodes;
    }

} // namespace CatchKit::Detail
