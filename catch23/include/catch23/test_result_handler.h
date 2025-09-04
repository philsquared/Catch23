//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_TEST_ASSERTION_HANDLER_H
#define CATCHKIT_TEST_ASSERTION_HANDLER_H

#include "internal_execution_nodes.h"
#include "reporter.h"
#include "catchkit/result_handler.h"

namespace CatchKit::Detail
{
    class TestCancelled {};

    enum class ShrinkingMode { Normal, Shrinking, Shrunk, NotShrunk };

    class TestResultHandler : public ResultHandler {
        Reporter& reporter;
        AssertionContext current_context;
        ResultType last_result = ResultType::Unknown;
        ResultDisposition result_disposition = ResultDisposition::Abort;
        ExecutionNodes* execution_nodes = nullptr;

        std::vector<VariableCapture*> variable_captures;
        Counters assertions;
        ShrinkingMode shrinking_mode = ShrinkingMode::Normal;
        int shrink_count = 0;

    public:
        explicit TestResultHandler(Reporter& reporter);

        auto operator=(TestResultHandler&&) = delete; // non-copyable, non-moveable

        void on_assertion_start( ResultDisposition result_disposition, AssertionContext&& context ) override;
        void on_assertion_result( ResultType result, ExpressionInfo const& expression_info, std::string_view message ) override;
        void on_assertion_end() override;

        void on_shrink_start();
        void on_shrink_found( std::vector<std::string> const& values );
        void on_shrink_end();

        void add_variable_capture( VariableCapture* capture ) override;
        void remove_variable_capture( VariableCapture* capture ) override;

        [[nodiscard]] auto get_reporter() -> Reporter& { return reporter; }
        [[nodiscard]] auto get_current_context() const -> AssertionContext const& { return current_context; }
        [[nodiscard]] auto passed() const { return last_result == ResultType::Passed; }
        [[nodiscard]] auto get_execution_nodes() const { return execution_nodes; }
        [[nodiscard]] auto get_assertion_counts() const { return assertions; }

        void reset_assertion_counts() { assertions = Counters(); }
        void set_execution_nodes( ExecutionNodes* nodes ) { execution_nodes = nodes; }
    };

    auto get_execution_nodes_from_result_handler(ResultHandler& handler) -> ExecutionNodes&;

} // namespace CatchKit::Detail

namespace CatchKit {

    using Detail::TestResultHandler;

} //namespace CatchKit

#endif // CATCHKIT_TEST_ASSERTION_HANDLER_H
