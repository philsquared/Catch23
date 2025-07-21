//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_ASSERT_RESULT_HANDLER_H
#define CATCHKIT_ASSERT_RESULT_HANDLER_H

#include "catchkit/catchkit_result_handler.h"

namespace CatchKit::Detail {

    struct AssertResultHandler : ResultHandler {
        AssertionContext const* current_context;
        ResultType last_result = ResultType::Unknown;
        void on_assertion_start( AssertionContext const& context ) override;
        void on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string const& message ) override;
        void on_assertion_end( ResultDisposition result_disposition ) override;
    };

    void set_current_assertion_handler( AssertResultHandler& handler ); // impl in catchkit_checker.cpp

} // namespace CatchKit::Detail

#endif //CATCHKIT_ASSERT_RESULT_HANDLER_H
