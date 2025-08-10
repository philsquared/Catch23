//
// Created by Phil Nash on 23/07/2025.
//

#ifndef CATCH23_INTERNAL_EXECUTION_NODES_H
#define CATCH23_INTERNAL_EXECUTION_NODES_H

#include <memory>
#include <vector>
#include <source_location>
#include <cassert>

#include "catchkit/stringify.h"

namespace CatchKit::Detail {

    struct NodeId {
        std::string name;
        std::source_location location = std::source_location::current();

        // !TBD: We could have a NodeIdLite that has a string_view so we don't copy the strings
        // every time

        auto operator == (NodeId const& other) const -> bool {
            return location.line() == other.location.line()
                && location.column() == other.location.column()
                && location.file_name() == other.location.file_name();
        };
    };

    class ExecutionNodes;

    struct ShrinkableNode {
        virtual void start_shrinking() = 0;
        virtual void rebase_shrink() = 0;
        virtual void stop_shrinking() = 0;
        virtual auto shrink() -> bool = 0;
        virtual auto current_value_as_string() -> std::string = 0;
    };

    class ExecutionNode {
    public:
        enum class States {
            None, // Just added
            NotEntered, // Added on a previous cycle but not (yet) entered on this one
            Entered,
            EnteredButDoneForThisLevel,
            ExitedEarly, // through an early return or exception - including test cancellation
            HasIncompleteChildren,
            Incomplete, // Children are complete, but there are more local levels (e.g. generator values)
            Frozen, // Held in place during a shrink
            Completed
        };

    private:
        friend class ExecutionNodes;

        NodeId id;
        ExecutionNodes* container = nullptr;
        ExecutionNode* parent = nullptr;
        std::vector<std::unique_ptr<ExecutionNode>> children;
        States state = States::None;

        auto get_current_node() -> ExecutionNode*;
        auto set_current_node(ExecutionNode* node);

    protected:
        ShrinkableNode* shrinkable = nullptr; // May be set by derived class
        std::size_t current_index = 0;
        virtual void move_first() {}
        virtual auto move_next() -> bool; // `true` means we finished
    public:
        explicit ExecutionNode( NodeId&& id )
        :   id(std::move(id))
        {}
        virtual ~ExecutionNode() = default;

        auto find_child(NodeId const& id) -> ExecutionNode*;

        auto add_child(std::unique_ptr<ExecutionNode>&& child) -> ExecutionNode& {
            child->parent = this;
            children.emplace_back( std::move(child) );
            return *children.back();
        }
        auto add_child(NodeId&& id) -> ExecutionNode&;

        [[nodiscard]] auto get_state() const { return state; }
        [[nodiscard]] auto get_parent() { return parent; }
        [[nodiscard]] auto get_parent_state() const { return parent ? parent->get_state() : States::None; }
        [[nodiscard]] auto get_current_index() const { return current_index; }
        [[nodiscard]] auto get_shrinkable() const { return shrinkable; }

        void reset();
        void reset_children();

        void enter();
        auto exit(bool early = false) -> States;

        auto freeze() -> States;
        void unfreeze(States state);
    };

    class ExecutionNodes {
        ExecutionNode root;
        ExecutionNode* current_node;
        friend class ExecutionNode;
    public:
        explicit ExecutionNodes(NodeId&& root_id)
        :   root(std::move(root_id)),
            current_node(&root)
        {
            root.container = this;
        }

        auto add_node(std::unique_ptr<ExecutionNode>&& child) -> ExecutionNode&;
        auto add_node(NodeId&& id) -> ExecutionNode&;

        [[nodiscard]] auto& get_root() { return root; }
        [[nodiscard]] auto get_current_node() const { return current_node; }
        [[nodiscard]] auto find_node(NodeId const& id) -> ExecutionNode* {
            return current_node->find_child(id);
        }
    };

} // namespace CatchKit::Detail


#endif // CATCH23_INTERNAL_EXECUTION_NODES_H
