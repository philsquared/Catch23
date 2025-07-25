//
// Created by Phil Nash on 23/07/2025.
//

#include "../include/catch23/catch23_internal_execution_nodes.h"


namespace CatchKit::Detail {

    auto ExecutionNode::get_current_node() -> ExecutionNode* {
        assert(container);
        return container->current_node;
    }
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
        return add_child( std::make_unique<ExecutionNode>(std::move(id)) );
    }

    void ExecutionNode::reset() {
        if( state != States::NotEntered ) {
            state = States::NotEntered;
            current_index = 0;
            reset_children();
        }
    }
    void ExecutionNode::reset_children() {
        for(auto const& child : children) {
            child->reset();
        }
    }

    void ExecutionNode::enter() {
        assert(state != States::Entered && state != States::Completed);
        state = States::Entered;
        set_current_node(this);
    }
    void ExecutionNode::skip() {
        assert(state == States::NotEntered || state == States::Completed || state == States::Skipped || state == States::None);
        state = States::Skipped;
    }
    auto ExecutionNode::exit(bool early) -> States {
        assert(state == States::Entered || state == States::EnteredButDoneForThisLevel);

        if(parent) {
            assert(parent->state == States::Entered || parent->state == States::EnteredButDoneForThisLevel);
            parent->state = States::EnteredButDoneForThisLevel;
        }
        for(auto const& child : children) {
            if( child->state == States::Entered || child->state == States::EnteredButDoneForThisLevel )
                child->exit();
            if( child->state != States::Completed && child->state != States::NotEntered ) {
                state = States::HasIncompleteChildren;
            }
            if( child->state == States::ExitedEarly )
                child->state = States::Completed;
        }
        if( get_current_node() == this ) {
            set_current_node(parent);
        }

        if( state == States::HasIncompleteChildren )
            return state;

        if( ++current_index != size ) {
            reset_children();
            return state = States::Incomplete;
        }

        reset_children();
        if( early )
            return state = States::ExitedEarly;

        return state = States::Completed;
    }

    bool ExecutionNode::move_next() {
        assert(current_index < size);
        return ++current_index <= size;
    }

    auto ExecutionNodes::add_node(std::unique_ptr<ExecutionNode>&& child) -> ExecutionNode& {
        child->container = this;
        return current_node->add_child(std::move(child));
    }

    auto ExecutionNodes::add_node(NodeId&& id) -> ExecutionNode& {
        assert(find_node(id) == nullptr);
        auto& new_node = current_node->add_child(std::move(id));
        new_node.container = this;
        return new_node;
    }

} // namespace CatchKit::Detail
