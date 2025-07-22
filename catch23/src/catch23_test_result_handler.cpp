//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/catch23_test_result_handler.h"
#include "catchkit/catchkit_assertion_context.h"

#include <print>
#include <cassert>

namespace CatchKit::Detail {

    TestResultHandler::TestResultHandler(std::unique_ptr<Reporter>&& reporter)
    : reporter(std::move(reporter))
    {}

    void TestResultHandler::on_assertion_start( ResultDisposition result_disposition, AssertionContext&& context ) {
        current_context = std::move(context);
        this->result_disposition = result_disposition;
        reporter->on_assertion_start( context );
    }

    void TestResultHandler::on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string_view message ) {;
        last_result = result;

        reporter->on_assertion_end(current_context, AssertionInfo{ result, expression_info, std::string(message) } );

    }
    void TestResultHandler::on_assertion_end() {
        if( last_result != ResultType::Pass && result_disposition == ResultDisposition::Abort ) {
            throw TestCancelled();
        }
    }

} // namespace CatchKit::Detail
