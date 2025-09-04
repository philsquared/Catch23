//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/assert_result_handler.h"
#include "catchkit/assertion_context.h"

#include <print>
#include <cassert>
#include <exception>

namespace CatchKit::Detail {

    void AssertResultHandler::on_assertion_start( ResultDisposition result_disposition, AssertionContext&& context ) {
        current_context = std::move(context);
        this->result_disposition = result_disposition;
    }

    void AssertResultHandler::on_assertion_result( ResultType result, ExpressionType expression_type, std::optional<ExpressionInfo> const& expression_info, std::string_view message ) {;
        last_result = result;

        // !TBD When we can use stacktrace do something like this:
        // https://godbolt.org/z/jM4TnaMEW
        if( last_result == ResultType::Passed )
            return;

        // Redundant?
        auto os = stdout;
        if (last_result != ResultType::Passed )
            os = stderr;

        std::println(os, "{}:{}:{}: in function '{}'",
                current_context.location.file_name(),
                current_context.location.line(),
                current_context.location.column(),
                current_context.location.function_name());
        std::string_view macro_name = current_context.macro_name;
        if ( macro_name.empty() )
            macro_name = "assertion";
        if ( !current_context.original_expression.empty() )
            std::println(os, "{} {} for expression:\n\t{}",
                macro_name, (result == ResultType::Passed) ? "passed" : "failed",
                current_context.original_expression);
        else
            std::println(os, "{} {}",
                macro_name, (result == ResultType::Passed) ? "passed" : "failed");

        if( result == ResultType::Failed ) {
            if( expression_type == ExpressionType::Exception ) {
                std::println(os, "due to an unexpected exception");
                // !TBD: distinguish
                // std::println(os, "due to a missing exception");
            // TBD: other types
            }
        }
        if(expression_info)
            std::println(os, "with expansion:\n\t{}", *expression_info );

        if (!message.empty()) {
            std::println(os, "with message:\n\t{}", message);
        }
    }
    void AssertResultHandler::on_assertion_end() {
        if ( last_result != ResultType::Passed && result_disposition == ResultDisposition::Abort ) {
            std::terminate();
        }
    }

} // namespace CatchKit::Detail
