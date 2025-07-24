//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23/catch23_sections.h"

#include "catch23/catch23_test_result_handler.h"

namespace CatchKit::Detail {

    SectionInfo::~SectionInfo() {
        if( entered )
            node.exit( std::uncaught_exceptions() > 0 );
    }

    auto try_enter_section(ExecutionNodes& nodes, std::string_view name, std::source_location const& location) -> SectionInfo {
        // !TBD: avoid always copying the string
        if(auto node = nodes.find_node({std::string(name), location})) {
            if( node->get_state() != ExecutionNode::States::Completed
                    && node->get_parent_state() != ExecutionNode::States::EnteredButDoneForThisLevel ) {
                node->enter();
                return SectionInfo{*node, true};
            }
            return SectionInfo{*node, false};
        }
        auto& node = nodes.add_node({std::string(name), location});
        if( node.get_parent_state() != ExecutionNode::States::EnteredButDoneForThisLevel ) {
            node.enter();
            return SectionInfo{node, true};
        }
        return SectionInfo{node, false};
    }
    auto try_enter_section(ResultHandler& handler, std::string_view name, std::source_location const& location) -> SectionInfo {
        auto execution_nodes = dynamic_cast<TestResultHandler&>(handler).get_execution_nodes();
        assert(execution_nodes);
        return try_enter_section(*execution_nodes, name, location);
    }


} // namespace CatchKit::Detail
