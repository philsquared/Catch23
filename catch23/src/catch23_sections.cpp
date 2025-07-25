//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23/catch23_sections.h"

#include "catch23/catch23_test_result_handler.h"

#include <exception>

namespace CatchKit::Detail {

    SectionInfo::~SectionInfo() {
        if( entered )
            node.exit( std::uncaught_exceptions() > 0 );
    }

    namespace {
        auto try_enter_internal(ExecutionNode& node) {

            // Don't enter if we've already entered a previous peer node
            if( node.get_parent_state() == ExecutionNode::States::EnteredButDoneForThisLevel ) {
                node.skip();
                return false;
            }

            // ... or if this node has already been completed
            if( node.get_state() == ExecutionNode::States::Completed ) {
                return false;
            }

            node.enter();
            return true;
        }
    }

    auto try_enter_section(ExecutionNodes& nodes, std::string_view name, std::source_location const& location) -> SectionInfo {
        // !TBD: avoid always copying the string
        auto node = nodes.find_node({std::string(name), location});
        if( !node )
            node = &nodes.add_node({std::string(name), location});
        return SectionInfo{ *node, try_enter_internal(*node) };
    }

} // namespace CatchKit::Detail
