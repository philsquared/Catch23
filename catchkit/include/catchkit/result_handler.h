//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_VIOLATION_HANDLER_H
#define CATCHKIT_VIOLATION_HANDLER_H

#include "result_type.h"
#include "expression_info.h"
#include "report_on.h"

#include <optional>
#include <string_view>

namespace CatchKit::Detail
{
    struct AssertionContext;
    struct VariableCapture;

    enum class ResultDisposition { Abort, Continue };

    struct ResultHandler {
        ReportOn report_on;

        explicit ResultHandler(ReportOn report_on) : report_on(report_on) {}
        virtual ~ResultHandler();

        virtual void on_assertion_start( ResultDisposition result_disposition, AssertionContext&& context ) = 0;
        virtual void on_assertion_result( ResultType result, ExpressionInfo const& expression_info, std::string_view message ) = 0;
        virtual void on_assertion_end() = 0;

        virtual void add_variable_capture(VariableCapture*) { /* do nothing by default */ };
        virtual void remove_variable_capture(VariableCapture*) { /* do nothing by default */ };

    };

} // namespace CatchKit::Detail

namespace CatchKit
{
    using Detail::ResultDisposition;
}

#endif // CATCHKIT_VIOLATION_HANDLER_H
