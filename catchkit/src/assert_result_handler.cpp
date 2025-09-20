//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/assert_result_handler.h"
#include "catchkit/assertion_context.h"

#include <print>
#include <cassert>
#include <exception>

namespace CatchKit::Detail {

    void AssertResultHandler::on_assertion_start( ResultDisposition result_disposition, AssertionContext const& context ) {
        current_context = std::move(context);
        this->result_disposition = result_disposition;
    }

    auto AssertResultHandler::on_assertion_result( ResultType result ) -> ResultDetailNeeded {
        last_result = result;
        if( last_result == ResultType::Passed )
            return ResultDetailNeeded::No;
        return ResultDetailNeeded::Yes;
    }
    void AssertResultHandler::on_assertion_result_detail( ExpressionInfo const& expression_info, std::string_view message ) {;
        // !TBD When we can use stacktrace do something like this:
        // https://godbolt.org/z/jM4TnaMEW

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
                macro_name, (last_result == ResultType::Passed) ? "passed" : "failed",
                current_context.original_expression);
        else
            std::println(os, "{} {}",
                macro_name, (last_result == ResultType::Passed) ? "passed" : "failed");

        if( last_result == ResultType::Failed ) {
            if( auto except_expr = std::get_if<ExceptionExpressionInfo>( &expression_info ) ) {
                switch( except_expr->type ) {
                    case ExceptionExpressionInfo::Type::Missing:
                        std::println(os, "due to a missing exception");
                        break;
                case ExceptionExpressionInfo::Type::Unexpected:
                    std::println(os, "due to an unexpected exception");
                    break;
                default:
                    assert( false );
                }
            }
        }
        if(!std::holds_alternative<std::monostate>(expression_info))
            std::println(os, "with expansion:\n\t{}", expression_info );

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
