//
// Created by Phil Nash on 23/07/2025.
//

#include "catch23/internal_execution_nodes.h"

namespace CatchKit::Detail {

    auto ExecutionNode::get_current_node() const -> ExecutionNode* {
        assert(container);
        return container->current_node;
    }
    auto ExecutionNode::set_current_node(ExecutionNode* node) { // NOLINT
        assert(container);
        return container->current_node = node;
    }

    auto ExecutionNode::find_child( NodeId const& id_to_find ) const -> ExecutionNode* {
        for(auto const& child : children) {
            if(child->id == id_to_find)
                return child.get();
        }
        return nullptr;
    }

    auto ExecutionNode::add_child(NodeId const& id_to_add) -> ExecutionNode& {
        return add_child( std::make_unique<ExecutionNode>(id_to_add) );
    }

    void ExecutionNode::reset() {
        if( state != States::NotEntered ) {
            state = States::NotEntered;
            current_index = 0;
            move_first();
            reset_children();
        }
    }
    void ExecutionNode::reset_children() { // NOLINT
        for(auto const& child : children) {
            child->reset();
        }
    }

    void ExecutionNode::enter() {
        assert(state != States::Entered && state != States::Completed);
        state = States::Entered;
        set_current_node(this);
    }

    auto ExecutionNode::move_next() -> bool {
        return ++current_index == 1;
    }

    auto ExecutionNode::exit(bool early) -> States {
        using enum States;
        assert(state == Entered || state == EnteredButDoneForThisLevel);

        if(parent) {
            assert(parent->state == Entered || parent->state == EnteredButDoneForThisLevel);
            parent->state = EnteredButDoneForThisLevel;
        }
        bool all_children_are_complete = true;
        for(auto const& child : children) {
            if( child->state == Entered || child->state == EnteredButDoneForThisLevel )
                child->exit();
            if( child->state != Completed && child->state != NotEntered ) {
                state = HasIncompleteChildren;
            }
            if( child->state == ExitedEarly )
                child->state = Completed;

            if( child->state != Completed )
                all_children_are_complete = false;
        }
        if( get_current_node() == this ) {
            set_current_node(parent);
        }

        if( state == HasIncompleteChildren )
            return state;

        if( state == EnteredButDoneForThisLevel && !all_children_are_complete )
            return state;

        assert( state == Entered || state == EnteredButDoneForThisLevel );

        if( !move_next() ) {
            reset_children();
            return state = Incomplete;
        }

        reset_children();
        if( early )
            return state = ExitedEarly;

        return state = Completed;
    }

    auto ExecutionNodes::add_node(std::unique_ptr<ExecutionNode>&& child) -> ExecutionNode& {
        child->container = this;
        return current_node->add_child(std::move(child));
    }

    auto ExecutionNodes::add_node(NodeId const& id) -> ExecutionNode& {
        assert(find_node(id) == nullptr);
        auto& new_node = current_node->add_child(id);
        new_node.container = this;
        return new_node;
    }

    auto ExecutionNode::freeze() -> States {
        States prev_state = state;
        state = States::Frozen;
        return prev_state;
    }
    void ExecutionNode::unfreeze(States prev_state) {
        state = prev_state;
    }

} // namespace CatchKit::Detail
