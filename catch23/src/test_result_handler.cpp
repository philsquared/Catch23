//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/test_result_handler.h"
#include "catchkit/assertion_context.h"

#include "catchkit/variable_capture.h"

#include <cassert>
#include <utility>

namespace CatchKit::Detail {

    TestResultHandler::TestResultHandler(Reporter& reporter)
    :   ResultHandler(reporter.report_on_what()),
        reporter(reporter)
    {}

    void TestResultHandler::on_test_start( TestInfo const& test_info ) {
        current_test_info = &test_info;
        reporter.on_test_start(test_info);
    }
    void TestResultHandler::on_test_end( TestInfo const& test_info ) {
        reporter.on_test_end(test_info, assertions);
        assertions = Counters();
        current_test_info = nullptr;
    }

    void TestResultHandler::on_assertion_start( ResultDisposition result_disposition, AssertionContext const& context ) {
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
        reporter.on_shrink_end();
    }
    auto TestResultHandler::on_assertion_result( ResultType result ) -> ResultDetailNeeded {
        if( current_test_info->should_fail() )
            last_result = (result == ResultType::Passed) ? AdjustedResult::Failed : AdjustedResult::Passed;
        else if( current_test_info->may_fail() && result == ResultType::Failed )
            last_result = AdjustedResult::FailedExpectly;
        else
            last_result = (result == ResultType::Passed) ? AdjustedResult::Passed : AdjustedResult::Failed;

        if( shrinking_mode == ShrinkingMode::Shrinking ) {
            shrink_count++;
            reporter.on_shrink_result(result, shrink_count);
            return ResultDetailNeeded::No;
        }
        if( shrinking_mode == ShrinkingMode::NotShrunk )
            return ResultDetailNeeded::No;

        // If we completed a shrink then we get called one more time so we report the details.
        // In that case we don't want to contribute to the assertion stats as will have already done so
        // before we started shrinking
        if( shrinking_mode != ShrinkingMode::Shrunk ) {
            switch(last_result) {
            case AdjustedResult::Passed:
                assertions.passed_explicitly++;
                break;
            case AdjustedResult::Failed:
                assertions.failed++;
                break;
            case AdjustedResult::FailedExpectly:
                assertions.failed_expectedly++;
                break;
            default:
                std::unreachable();
            }
        }

        if( report_on != ReportOn::AllResults &&
                last_result != AdjustedResult::Failed &&
                !current_test_info->has_tag_type( Tag::Type::always_report ) )
            return ResultDetailNeeded::No;

        return ResultDetailNeeded::Yes;
    }

    void TestResultHandler::on_assertion_result_detail( ExpressionInfo const& expression_info, std::string_view message ) {
        assert(current_test_info);

        if( !message.empty() ) {
            // Attempt to strip out an inline message
            // note: this is quite brittle, so if it seems to have stopped working
            // check that this logic still matches usage
            std::string_view& expr = current_context->original_expression;
            if( auto pos = expr.rfind(message); pos != std::string::npos ) {
                assert( pos > 3 && expr[pos-3] == ',' );
                expr = expr.substr( 0, pos-3 );
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
        reporter.on_assertion_end(*current_context, AssertionInfo{ last_result, expression_info, full_message } );
    }

    void TestResultHandler::on_assertion_end() {
        current_context.reset();
        if( !passed() && result_disposition == ResultDisposition::Abort ) {
            throw TestCancelled();
        }
    }

    void TestResultHandler::add_variable_capture( VariableCapture* capture ) {
        variable_captures.push_back(capture);
    }
    void TestResultHandler::remove_variable_capture( VariableCapture* capture ) {
        std::erase(variable_captures, capture);
    }

    auto TestResultHandler::get_last_known_location() const -> std::source_location {
        assert( current_test_info );
        if( current_context )
            return current_context->location;
        return current_test_info->location;
    }

    auto get_execution_nodes_from_result_handler(ResultHandler& handler) -> ExecutionNodes& {
        assert(dynamic_cast<TestResultHandler*>(&handler) != nullptr);
        auto execution_nodes = static_cast<TestResultHandler&>(handler).get_execution_nodes();
        assert(execution_nodes);
        return *execution_nodes;
    }

} // namespace CatchKit::Detail
