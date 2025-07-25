//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23/sections.h"

#include "catch23/test_result_handler.h"

#include <exception>

namespace CatchKit::Detail {

    SectionInfo::~SectionInfo() {
        if( entered )
            node.exit( std::uncaught_exceptions() > 0 );
    }

    auto try_enter_section(ExecutionNodes& nodes, std::string_view name, std::source_location const& location) -> SectionInfo {
        // !TBD: avoid always copying the string
        auto node = nodes.find_node({std::string(name), location});
        if( !node )
            node = &nodes.add_node({std::string(name), location});

        // Don't enter if we've already entered a previous peer node
        // or if this node has already been completed
        if( node->get_parent_state() == ExecutionNode::States::EnteredButDoneForThisLevel ||
            node->get_state() == ExecutionNode::States::Completed ) {
            return SectionInfo{ *node, false };
        }
        node->enter();
        return SectionInfo{ *node, true };
    }

} // namespace CatchKit::Detail
