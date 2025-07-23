//
// Created by Phil Nash on 23/07/2025.
//

#ifndef CATCH23_INTERNAL_EXECUTION_NODES_H
#define CATCH23_INTERNAL_EXECUTION_NODES_H

#include <memory>
#include <vector>
#include <source_location>
#include <cassert>

#include "catchkit/catchkit_stringify.h"

namespace CatchKit::Detail {

    struct NodeId {
        std::string name;
        std::source_location location = std::source_location::current();

        // !TBD: We could have a NodeIdLite that has a string_view so we don't copy the strings
        // every time

        auto operator == (NodeId const& other) const -> bool {
            return location.line() == other.location.line()
                && name == other.name
                && location.file_name() == other.location.file_name();
        };
    };

    class ExecutionNodes;

    class ExecutionNode {
    public:
        enum class States {
            None, // Just added or not entered yet
            Entered,
            EnteredButDoneForThisLevel,
            HasIncompleteChildren,
            Incomplete, // Children are complete, but there are more local levels (e.g. generator values)
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
        const size_t size;
        size_t current_index = 0;

    public:
        explicit ExecutionNode(NodeId&& id, size_t size = 1)
        :   id(std::move(id)),
            size(size)
        {}
        virtual ~ExecutionNode() = default;

        auto find_child(NodeId const& id) -> ExecutionNode*;

        void add_child(std::unique_ptr<ExecutionNode>&& child) {
            child->parent = this;
            children.emplace_back( std::move(child) );
        }
        auto add_child(NodeId&& id) -> ExecutionNode&;

        auto get_state() const { return state; }
        auto get_parent() { return parent; }
        auto get_parent_state() const { return parent ? parent->get_state() : States::None; }
        auto get_size() const { return size; }
        auto get_current_index() const { return current_index; }

        void reset();
        void reset_children();

        void enter();
        auto exit() -> States;

        bool move_next();
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

        auto find_node(NodeId const& id) -> ExecutionNode* {
            return current_node->find_child(id);
        }
        void add_node(std::unique_ptr<ExecutionNode>&& child);
        auto add_node(NodeId&& id) -> ExecutionNode&;

        auto& get_root() { return root; }
    };

} // namespace CatchKit::Detail


#endif //CATCH23_INTERNAL_EXECUTION_NODES_H
