//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_TEST_ASSERTION_HANDLER_H
#define CATCHKIT_TEST_ASSERTION_HANDLER_H

#include "catch23_internal_execution_nodes.h"
#include "catch23_reporter.h"
#include "catchkit/catchkit_result_handler.h"

namespace CatchKit::Detail
{
    class TestCancelled {};

    class TestResultHandler : public ResultHandler {
        Reporter& reporter;
        AssertionContext current_context;
        ResultType last_result = ResultType::Unknown;
        ResultDisposition result_disposition = ResultDisposition::Abort;
        ExecutionNodes* execution_nodes = nullptr;

    public:
        explicit TestResultHandler(Reporter& reporter);

        auto operator=(TestResultHandler&&) = delete; // non-copyable, non-moveable

        void on_assertion_start( ResultDisposition result_disposition, AssertionContext&& context ) override;
        void on_assertion_result( ResultType result, std::optional<ExpressionInfo> const& expression_info, std::string_view message ) override;
        void on_assertion_end() override;

        [[nodiscard]] auto get_reporter() -> Reporter& { return reporter; }
        [[nodiscard]] auto get_current_context() const -> AssertionContext const& { return current_context; }
        [[nodiscard]] auto passed() const { return last_result == CatchKit::ResultType::Pass; }
        [[nodiscard]] auto get_execution_nodes() const { return execution_nodes; }

        void set_execution_nodes( ExecutionNodes* nodes ) { execution_nodes = nodes; }
    };

} // namespace CatchKit::Detail

namespace CatchKit {

    using Detail::TestResultHandler;

} //namespace CatchKit

#endif //CATCHKIT_TEST_ASSERTION_HANDLER_H
