//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/test_result_handler.h"
#include "catchkit/assertion_context.h"

#include <print>
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

    void TestResultHandler::on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string_view message ) {
        last_result = result;
        // !TBD: We should need to check this again
        // - go back to having two on_assertion_result methods - one that takes just a result,
        // the other takes the full, expanded, data (probably no need for optional)
        if( report_on == ReportOn::AllResults || result != ResultType::Pass ) {
            if( !message.empty() ) {
                // Attempt to string out an inline message
                // note: this is quite brittle, so if it seems to have stopped working
                // check that this logic still matches usage
                if( auto pos = current_context.original_expression.find(message); pos != std::string::npos ) {
                    if( pos > 3 && current_context.original_expression[pos-3] == ',' )
                        current_context.original_expression = current_context.original_expression.substr( 0, pos-3 );
                }
            }
            reporter.on_assertion_end(current_context, AssertionInfo{ result, expression_info, std::string(message) } );
        }
    }

    void TestResultHandler::on_assertion_end() {
        if( last_result != ResultType::Pass && result_disposition == ResultDisposition::Abort ) {
            throw TestCancelled();
        }
    }

    auto get_execution_nodes_from_result_handler(ResultHandler& handler) -> ExecutionNodes& {
        assert(dynamic_cast<TestResultHandler*>(&handler) != nullptr);
        auto execution_nodes = static_cast<TestResultHandler&>(handler).get_execution_nodes();
        assert(execution_nodes);
        return *execution_nodes;
    }

} // namespace CatchKit::Detail
