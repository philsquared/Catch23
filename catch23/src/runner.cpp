//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/runner.h"
#include "catch23/internal_execution_nodes.h"

namespace CatchKit::Detail {

    namespace {
        void invoke_test( Test const& test, TestResultHandler& test_handler ) {
            Checker check{
                .result_handler=&test_handler,
                .result_disposition=ResultDisposition::Continue };
            Checker require{
                .result_handler=&test_handler,
                .result_disposition=ResultDisposition::Abort };

            Checker old_check = ::check, old_require = ::require;
            ::check = check;
            ::require = require;

            try {
                test.test_fun(check, require);
            }
            catch( TestCancelled ) {
            }
            catch( ... ) {
                // We need a new context because the old one had string_views to outdated data
                // - we want to preserve the last known source location, though
                AssertionContext context{
                    .macro_name = "",
                    .original_expression = "* unknown line after the reported location *",
                    .message = {},
                    .location = test_handler.get_current_context().location };
                test_handler.on_assertion_start( ResultDisposition::Continue, std::move(context) );
                test_handler.on_assertion_result( ResultType::UnexpectedException, {}, get_exception_message(std::current_exception()) );
            }
            ::check = old_check;
            ::require = old_require;
        }
    }
    auto try_shrink( Test const& test, TestResultHandler& test_handler, ExecutionNode* leaf_node ) {

        std::vector<ShrinkableNode*> shrinkables;
        auto node = leaf_node;
        for(; node->get_parent(); node = node->get_parent())
            if( auto shrinkable = node->get_shrinkable())
                shrinkables.push_back( shrinkable );

        if( shrinkables.empty() )
            return;

        test_handler.on_shrink_start();

        auto& root_node = *node;

        leaf_node->freeze();
        root_node.exit();
        std::vector<std::string> shrunk_values;
        shrunk_values.reserve( shrinkables.size() );
        for( auto& shrinkable : shrinkables ) {
            shrinkable->start_shrinking();
            while( shrinkable->shrink() ) {
                root_node.enter();

                invoke_test(test, test_handler);

                if(!test_handler.passed())
                    shrinkable->rebase_shrink(); // Resets on current failing number

                leaf_node->freeze();
                root_node.exit();
            }
            if( shrinkable->stop_shrinking() )
                shrunk_values.push_back( shrinkable->current_value_as_string() );
        }
        test_handler.on_shrink_found(shrunk_values);

        root_node.enter();
        invoke_test(test, test_handler);
        // don't do final exit as it will happen in caller
        test_handler.on_shrink_end();

    }

    void run_test( Test const& test, TestResultHandler& test_handler ) {
        Reporter& reporter = test_handler.get_reporter();

        ExecutionNodes execution_nodes({test.test_info.name, test.test_info.location});
        auto& root_node = execution_nodes.get_root();
        test_handler.set_execution_nodes(&execution_nodes);

        do {
            reporter.on_test_start(test.test_info);

            root_node.enter();
            assert(root_node.get_state() != ExecutionNode::States::Completed);

            invoke_test(test, test_handler);

            auto current_execution_node = execution_nodes.get_current_node();
            if( !test_handler.passed() )
                try_shrink(test, test_handler, current_execution_node);

            root_node.exit();


            reporter.on_test_end(test.test_info, test_handler.get_assertion_counts() );
            test_handler.reset_assertion_counts();

        }
        while(root_node.get_state() != ExecutionNode::States::Completed);

        test_handler.set_execution_nodes(nullptr);
    }

} // namespace CatchKit::Detail