//
// Created by Phil Nash on 23/07/2025.
//

#ifndef CATCH23_INTERNAL_EXECUTION_NODES_H
#define CATCH23_INTERNAL_EXECUTION_NODES_H

#include <memory>
#include <vector>
#include <source_location>
#include <cassert>

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
            ExitedButIncomplete,
            Completed
        };

    private:
        NodeId id;
        ExecutionNodes& container;
        ExecutionNode* parent;
        std::vector<std::unique_ptr<ExecutionNode>> children;
        States state = States::None;

    public:
        explicit ExecutionNode(NodeId&& id, ExecutionNodes& container, ExecutionNode* parent)
        :   id(std::move(id)),
            container(container),
            parent(parent)
        {}

        auto find_child(NodeId const& id) -> ExecutionNode*;
        auto add_child(NodeId&& id) -> ExecutionNode&;

        auto get_state() const { return state; }
        auto get_parent() { return parent; }
        auto get_parent_state() const { return parent ? parent->get_state() : States::None; }

        void reset();
        void enter();
        auto exit() -> States;
    };

    class ExecutionNodes {
        ExecutionNode root;
        ExecutionNode* current_node;
        friend class ExecutionNode;
    public:
        explicit ExecutionNodes(NodeId&& root_id)
        :   root(std::move(root_id), *this, nullptr),
            current_node(&root)
        {}

        auto find_node(NodeId const& id) -> ExecutionNode* {
            return current_node->find_child(id);
        }
        auto add_node(NodeId&& id) -> ExecutionNode& {
            assert(find_node(id) == nullptr);
            return current_node->add_child(std::move(id));
        }

        auto& get_root() { return root; }
    };

} // namespace CatchKit::Detail

#endif //CATCH23_INTERNAL_EXECUTION_NODES_H
