//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/catch23_test_result_handler.h"
#include "catchkit/catchkit_assertion_context.h"

#include <print>
#include <cassert>

namespace CatchKit::Detail {
    void TestResultHandler::on_assertion_start( AssertionContext const& context ) {
        current_context = &context;
    }

    void TestResultHandler::on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string const& message ) {;
        assert(current_context);
        last_result = result;

        // !TBD: we don't really want to do this, here, but in a reporter:
        std::println("{}:{}:{}: in function '{}'",
                current_context->location.file_name(),
                current_context->location.line(),
                current_context->location.column(),
                current_context->location.function_name());
        std::println("{} {} for expression:", current_context->macro_name, (result == ResultType::Pass) ? "passed" : "failed");
        std::println("\t{}", current_context->original_expression);
        switch( result ) {
        case ResultType::UnexpectedException:
            std::println("due to an unexpected exception");
            break;
        case ResultType::MissingException:
            std::println("due to a missing exception");
            break;
        default:
            if(expression_info)
                std::println("with expansion:\n\t{}", *expression_info);
            break;
        }
        if (!message.empty()) {
            std::println("with message:\n\t{}", message);
        }
    }
    void TestResultHandler::on_assertion_end(ResultDisposition result_disposition) {
        if( last_result != ResultType::Pass && result_disposition == ResultDisposition::Abort ) {
            throw std::domain_error("cancel test"); // !TBD
        }

        current_context = nullptr;
    }

} // namespace CatchKit::Detail
