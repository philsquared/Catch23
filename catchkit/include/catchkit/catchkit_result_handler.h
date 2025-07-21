//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_VIOLATION_HANDLER_H
#define CATCHKIT_VIOLATION_HANDLER_H

#include "catchkit_result_type.h"
#include "catchkit_expression_info.h"

#include <optional>
#include <string>

namespace CatchKit::Detail
{
    struct AssertionContext;

    enum class ResultDisposition { Abort, Continue };

    struct ResultHandler {
        virtual ~ResultHandler();
        virtual void on_assertion_start( AssertionContext const& context ) = 0;
        virtual void on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string const& message ) = 0;
        virtual void on_assertion_end( ResultDisposition result_disposition ) = 0;
    };

} // namespace CatchKit::Detail

namespace CatchKit
{
    using Detail::ResultDisposition;
}

#endif //CATCHKIT_VIOLATION_HANDLER_H
