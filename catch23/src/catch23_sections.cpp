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

    auto try_enter_section(ExecutionNodes& nodes, std::string_view name, std::source_location const& location) -> SectionInfo {
        // !TBD: avoid always copying the string
        if(auto node = nodes.find_node({std::string(name), location})) {
            if( node->get_parent_state() == ExecutionNode::States::EnteredButDoneForThisLevel ) {
                node->skip();
                return SectionInfo{*node, false};
            }
            else if( node->get_state() == ExecutionNode::States::Completed ) {

                // !TBD Deduplicate this logic
                return SectionInfo{*node, false};
            }
            else {
                node->enter();
                return SectionInfo{*node, true};
            }
        }
        auto& node = nodes.add_node({std::string(name), location});
        if( node.get_parent_state() != ExecutionNode::States::EnteredButDoneForThisLevel ) {
            node.enter();
            return SectionInfo{node, true};
        }
        node.skip();
        return SectionInfo{node, false};
    }

} // namespace CatchKit::Detail
