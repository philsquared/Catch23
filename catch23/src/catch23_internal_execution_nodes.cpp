//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23_internal_execution_nodes.h"

namespace CatchKit::Detail {

    auto ExecutionNode::set_current_node(ExecutionNode* node) {
        assert(container);
        return container->current_node = node;
    }

    auto ExecutionNode::find_child(NodeId const& id) -> ExecutionNode* {
        for(auto const& child : children) {
            if(child->id == id)
                return child.get();
        }
        return nullptr;
    }

    auto ExecutionNode::add_child(NodeId&& id) -> ExecutionNode& {
        add_child( std::make_unique<ExecutionNode>(std::move(id)) );
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
        set_current_node(this);
    }
    auto ExecutionNode::exit() -> States {
        assert(state == States::Entered || state == States::EnteredButDoneForThisLevel);

        if(parent) {
            set_current_node(parent);
            assert(parent->state == States::Entered || parent->state == States::EnteredButDoneForThisLevel);
            parent->state = States::EnteredButDoneForThisLevel;
        }
        for(auto const& child : children) {
            if( child->state != States::Completed )
                return state = States::ExitedButIncomplete;
        }

        return state = States::Completed;
    }

    bool ExecutionNode::move_next() {
        assert(current_index < size);
        return ++current_index <= size;
    }

    void ExecutionNodes::add_node(std::unique_ptr<ExecutionNode>&& child) {
        child->container = this;
        current_node->add_child(std::move(child));
    }

    auto ExecutionNodes::add_node(NodeId&& id) -> ExecutionNode& {
        assert(find_node(id) == nullptr);
        auto& new_node = current_node->add_child(std::move(id));
        new_node.container = this;
        return new_node;
    }

} // namespace CatchKit::Detail
