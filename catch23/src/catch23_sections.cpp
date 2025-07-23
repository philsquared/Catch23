//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23/catch23_sections.h"

namespace CatchKit::Detail {

    auto try_enter_section(ExecutionNodes& nodes, std::string&& name, std::source_location const& location) -> SectionInfo {
        if(auto node = nodes.find_node({name, location})) {
            if(node->get_state() != ExecutionNode::States::Completed
                && node->get_parent_state() != ExecutionNode::States::EnteredButDoneForThisLevel) {
                node->enter();
                return SectionInfo{*node, true};
                }
            return SectionInfo{*node, false};
        }
        auto& node = nodes.add_node({std::move(name), location});
        if( node.get_parent_state() != ExecutionNode::States::EnteredButDoneForThisLevel ) {
            node.enter();
            return SectionInfo{node, true};
        }
        return SectionInfo{node, false};
    }

} // namespace CatchKit::Detail