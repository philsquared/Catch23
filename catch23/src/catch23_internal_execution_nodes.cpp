//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23_internal_execution_nodes.h"

namespace CatchKit::Detail {

    auto ExecutionNode::find_child(NodeId const& id) -> ExecutionNode* {
        for(auto const& child : children) {
            if(child->id == id)
                return child.get();
        }
        return nullptr;
    }

    auto ExecutionNode::add_child(NodeId&& id) -> ExecutionNode& {
        children.emplace_back( std::make_unique<ExecutionNode>(std::move(id), container, this) );
        return *children.back();
    }

    void ExecutionNode::reset() {
        assert(state != States::Entered);
        for(auto const& child : children) {
            child->reset();
        }
    }

    void ExecutionNode::enter() {
        assert(state != States::Entered);
        reset();
        state = States::Entered;
        container.current_node = this;
    }
    auto ExecutionNode::exit() -> States {
        assert(state == States::Entered || state == States::EnteredButDoneForThisLevel);

        if(parent) {
            container.current_node = parent;
            assert(parent->state == States::Entered || parent->state == States::EnteredButDoneForThisLevel);
            parent->state = States::EnteredButDoneForThisLevel;
        }
        for(auto const& child : children) {
            if( child->state != States::Completed )
                return state = States::ExitedButIncomplete;
        }

        return state = States::Completed;
    }

} // namespace CatchKit::Detail
