//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_TEST_ASSERTION_HANDLER_H
#define CATCHKIT_TEST_ASSERTION_HANDLER_H

#include <memory>

#include "catch23_reporter.h"
#include "catchkit/catchkit_result_handler.h"

namespace CatchKit::Detail
{
    class TestResultHandler : public ResultHandler {
        std::unique_ptr<Reporter> reporter;
        AssertionContext const* current_context;
        ResultType last_result = ResultType::Unknown;
        ResultDisposition result_disposition = ResultDisposition::Abort;

        void on_assertion_start( ResultDisposition result_disposition, AssertionContext const& context ) override;
        void on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string_view message ) override;
        void on_assertion_end() override;

    public:
        explicit TestResultHandler(std::unique_ptr<Reporter>&& reporter);
        auto get_reporter() -> Reporter& { return *reporter; }
        auto passed() { return last_result == CatchKit::ResultType::Pass; }
    };

} // namespace CatchKit::Detail

namespace CatchKit {

    using Detail::TestResultHandler;

} //namespace CatchKit

#endif //CATCHKIT_TEST_ASSERTION_HANDLER_H
