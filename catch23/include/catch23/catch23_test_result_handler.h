//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_TEST_ASSERTION_HANDLER_H
#define CATCHKIT_TEST_ASSERTION_HANDLER_H

#include "catchkit/catchkit_result_handler.h"

namespace CatchKit::Detail
{
    struct TestResultHandler : ResultHandler {
        AssertionContext const* current_context;
        ResultType last_result = ResultType::Unknown;
        void on_assertion_start( AssertionContext const& context ) override;
        void on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string const& message ) override;
        void on_assertion_end( ResultDisposition result_disposition ) override;
    };

} // namespace CatchKit::Detail

namespace CatchKit
{
    using Detail::TestResultHandler;

} //namespace CatchKit

#endif //CATCHKIT_TEST_ASSERTION_HANDLER_H
