//
// Created by Phil Nash on 22/07/2025.
//

#include "../include/catch23/runner.h"
#include "../include/catch23/internal_execution_nodes.h"

namespace CatchKit::Detail {

    void run_test( Test const& test, TestResultHandler& test_handler ) {
        Reporter& reporter = test_handler.get_reporter();

        ExecutionNodes execution_nodes({test.test_info.name, test.test_info.location});
        auto& root_node = execution_nodes.get_root();
        test_handler.set_execution_nodes(&execution_nodes);

        do {
            root_node.enter();
            assert(root_node.get_state() != ExecutionNode::States::Completed);

            reporter.on_test_start(test.test_info);

            Checker check{
                .result_handler=test_handler,
                .result_disposition=ResultDisposition::Continue };
            Checker require{
                .result_handler=test_handler,
                .result_disposition=ResultDisposition::Abort };

            try {
                test.test_fun(check, require);
            }
            catch( TestCancelled ) {
                // std::println("  *** aborted"); // !TBD
            }
            catch( ... ) {
                // We need a new context because the old one had string_views to outdated data
                // - we want to preserve the last known source location, though
                AssertionContext context{
                    .macro_name = "",
                    .original_expression = "* unknown line after the reported location *",
                    .message = {},
                    .location = test_handler.get_current_context().location };
                test_handler.on_assertion_start( CatchKit::ResultDisposition::Continue, std::move(context) );
                test_handler.on_assertion_result( CatchKit::ResultType::UnexpectedException, {}, CatchKit::Detail::get_exception_message(std::current_exception()) );
            }
            root_node.exit();

            reporter.on_test_end(test.test_info, test_handler.get_assertion_counts() );
            test_handler.reset_assertion_counts();
        }
        while(root_node.get_state() != ExecutionNode::States::Completed);

        test_handler.set_execution_nodes(nullptr);
    }

} // namespace CatchKit::Detail