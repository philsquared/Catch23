//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/catchkit_assert_result_handler.h"
#include "catchkit/catchkit_assertion_context.h"

#include <print>
#include <cassert>
#include <exception>

namespace CatchKit::Detail {

    void AssertResultHandler::on_assertion_start( AssertionContext const& context ) {
        current_context = &context;
    }

    void AssertResultHandler::on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string const& message ) {;
        last_result = result;

        // !TBD When we can use stacktrace do something like this:
        // https://godbolt.org/z/jM4TnaMEW

        assert(current_context);

        auto os = stdout;
        if (last_result != ResultType::Pass )
            os = stderr;

        std::println(os, "{}:{}:{}: in function '{}'",
                current_context->location.file_name(),
                current_context->location.line(),
                current_context->location.column(),
                current_context->location.function_name());
        std::string_view macro_name = current_context->macro_name;
        if ( macro_name.empty() )
            macro_name = "assertion";
        if ( !current_context->original_expression.empty() )
            std::println(os, "{} {} for expression:\n\t{}",
                macro_name, (result == ResultType::Pass) ? "passed" : "failed",
                current_context->original_expression);
        else
            std::println(os, "{} {}",
                macro_name, (result == ResultType::Pass) ? "passed" : "failed");
        switch( result ) {
            case ResultType::UnexpectedException:
                std::println(os, "due to an unexpected exception");
                break;
            case ResultType::MissingException:
                std::println(os, "due to a missing exception");
                break;
            default:
                if(expression_info)
                    std::println(os, "with expansion:\n\t{}", *expression_info );
                break;
        }
        if (!message.empty()) {
            std::println(os, "with message:\n\t{}", message);
        }
    }
    void AssertResultHandler::on_assertion_end(ResultDisposition result_disposition) {
        if ( last_result != ResultType::Pass && result_disposition == ResultDisposition::Abort ) {
            std::terminate();
        }
        current_context = nullptr;
    }
} // namespace CatchKit::Detail
