//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_ASSERT_RESULT_HANDLER_H
#define CATCHKIT_ASSERT_RESULT_HANDLER_H

#include "catchkit/result_handler.h"
#include "catchkit/assertion_context.h"
#include "catchkit/result_type.h"

namespace CatchKit::Detail {

    struct AssertResultHandler : ResultHandler {
        AssertionContext current_context;
        ResultType last_result = ResultType::Passed;
        ResultDisposition result_disposition = ResultDisposition::Abort;

        AssertResultHandler() : ResultHandler(ReportOn::FailuresOnly) {}

        void on_assertion_start( ResultDisposition result_disposition, AssertionContext const& context ) override;
        [[nodiscard]] auto on_assertion_result( ResultType result ) -> ResultDetailNeeded override;
        void on_assertion_result_detail( ExpressionInfo const& expression_info, std::string_view message ) override;
        void on_assertion_end() override;
    };

} // namespace CatchKit::Detail

#endif // CATCHKIT_ASSERT_RESULT_HANDLER_H
