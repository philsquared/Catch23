//
// Created by Phil Nash on 23/07/2025.
//
#include "catch23/catch23_test.h"
#include "../src/catch23_internal_execution_nodes.h"

TEST("execution nodes") {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    nodes.get_root().enter();

    NodeId a_id({"a"}); // Use this one so it has a stable location

    CHECK( nodes.find_node(a_id) == nullptr );
    auto& node = nodes.add_node(NodeId(a_id));

    CHECK( nodes.find_node({"a"}) != &node,
        "we shouldn't find this node because it has a different loc");

    CHECK( nodes.find_node(NodeId(a_id)) == &node );

    node.enter();
    CHECK(node.get_state() == ExecutionNode::States::Entered);

    CHECK(node.exit() == ExecutionNode::States::Completed,
        "When we exit immediately should completes");

    CHECK(nodes.get_root().exit() == ExecutionNode::States::Completed);

    nodes.get_root().reset();

    nodes.get_root().enter();

    node.enter();
    CHECK(node.get_state() == ExecutionNode::States::Entered);

    NodeId b_id({"b"});

    auto& node_b = nodes.add_node(NodeId(b_id));
    CHECK(node.find_child(b_id) == &node_b);

    CHECK(node.exit() == ExecutionNode::States::Incomplete,
        "When we exit after adding a new node it should be incomplete");

    CHECK(nodes.get_root().exit() == ExecutionNode::States::Incomplete);
    nodes.get_root().enter();

    node.enter();
    node_b.enter();
    CHECK(node_b.exit() == ExecutionNode::States::Completed);
    CHECK(node.exit() == ExecutionNode::States::Completed);
    CHECK(nodes.get_root().exit() == ExecutionNode::States::Completed);
}

struct TickTockNode : public CatchKit::Detail::ExecutionNode {
    explicit TickTockNode(CatchKit::Detail::NodeId&& id)
    :   ExecutionNode(std::move(id), 2)
    {}

    int current_value() {
        return current_index + 1;
    }
};

TEST("TickTick Execution Nodes") {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    auto& root = nodes.get_root();
    CHECK( root.get_size() == 1 );

    root.enter();

    NodeId a_id({"a"}); // Use this one so it has a stable location

    CHECK( nodes.find_node(a_id) == nullptr );
    nodes.add_node( std::make_unique<TickTockNode>(NodeId(a_id)) );

    auto node = nodes.find_node(a_id);
    REQUIRE( node );
    CHECK( node->get_size() == 2 );

    node->enter();
    auto ttnode = dynamic_cast<TickTockNode*>(node);
    REQUIRE( ttnode );

    CHECK( ttnode->current_value() == 1 );

    node->exit();
    root.exit();

    root.enter();
    node->enter();

    // !TBD: Just need to get this working.
    // This will require advancing the "current_index" member
    // - but being careful of the interplay between that and child nodes
    CHECK( ttnode->current_value() == 3 );
}

namespace CatchKit::Detail {
    struct SectionInfo {
        ExecutionNode& node;
        bool entered;

        ~SectionInfo() {
            if( entered )
                node.exit();
        }

        explicit operator bool() const noexcept { return entered; }
    };

    auto try_enter_section(ExecutionNodes& nodes, std::string&& name, std::source_location const& location) {
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
}

TEST("peer sections") {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    nodes.get_root().enter();

    std::source_location stable_loc;

    CHECK( try_enter_section(nodes, "s1", stable_loc ),
        "should enter first node encountered");
    CHECK( !try_enter_section(nodes, "s2", stable_loc ),
        "should skip second node at same level");

    CHECK( nodes.get_root().exit() == ExecutionNode::States::Incomplete );
    nodes.get_root().enter(); // re-enter whole test

    CHECK( !try_enter_section(nodes, "s1", stable_loc ),
        "should skip first node when re-encountered");

    CHECK( try_enter_section(nodes, "s2", stable_loc ),
        "should now go into second node");
}

TEST("nested sections") {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    nodes.get_root().enter();

    std::source_location stable_loc;

    {
        auto s1 = try_enter_section(nodes, "s1", stable_loc);
        CHECK( s1, "should enter first node encountered");

        // section still open

        CHECK( try_enter_section(nodes, "s1.1", stable_loc ),
            "should enter nested node");

        CHECK( !try_enter_section(nodes, "s1.2", stable_loc ),
            "should skip subsequent node at nested level");
    }
    CHECK( !try_enter_section(nodes, "s2", stable_loc ),
        "should skip subsequent node at top level");

    CHECK( nodes.get_root().exit() == ExecutionNode::States::Incomplete );
    nodes.get_root().enter(); // re-enter whole test

    {
        auto s1 = try_enter_section(nodes, "s1", stable_loc);
        CHECK( s1, "should enter first node, again");

        // section still open

        CHECK( !try_enter_section(nodes, "s1.1", stable_loc ),
            "should now skip first nested node");

        CHECK( try_enter_section(nodes, "s1.2", stable_loc ),
            "should enter second nested node");
    }
    CHECK( !try_enter_section(nodes, "s2", stable_loc ),
        "should still skip subsequent node at top level");

    CHECK( nodes.get_root().exit() == ExecutionNode::States::Incomplete );
    nodes.get_root().enter(); // re-enter whole test

    CHECK( !try_enter_section(nodes, "s1", stable_loc), "should now skip first node");

    CHECK( try_enter_section(nodes, "s2", stable_loc ),
        "should now enter second node at top level");
}
