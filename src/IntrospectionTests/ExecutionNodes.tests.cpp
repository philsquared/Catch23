//
// Created by Phil Nash on 23/07/2025.
//
#include "catch23/catch23_test.h"
#include "catch23/catch23_sections.h"
#include "catch23/catch23_internal_execution_nodes.h"

TEST("execution nodes") {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    nodes.get_root().enter();

    NodeId a_id( {"a"} ); // Use this one so it has a stable location

    CHECK( nodes.find_node(a_id) == nullptr );
    auto& node = nodes.add_node(NodeId(a_id));

    CHECK( nodes.find_node({"a"}) != &node,
        "we shouldn't find this node because it has a different loc");

    CHECK( nodes.find_node(NodeId(a_id)) == &node );

    node.enter();
    CHECK( node.get_state() == ExecutionNode::States::Entered );

    CHECK( node.exit() == ExecutionNode::States::Completed,
        "When we exit immediately should completes" );

    CHECK( nodes.get_root().exit() == ExecutionNode::States::Completed );

    nodes.get_root().reset();

    nodes.get_root().enter();

    node.enter();
    CHECK( node.get_state() == ExecutionNode::States::Entered );

    NodeId b_id( {"b"} );

    auto& node_b = nodes.add_node( NodeId(b_id) );
    CHECK( node.find_child(b_id) == &node_b );

    CHECK( node.exit() == ExecutionNode::States::HasIncompleteChildren,
        "When we exit after adding a new node it should be incomplete" );

    CHECK( nodes.get_root().exit() == ExecutionNode::States::HasIncompleteChildren );
    nodes.get_root().enter();

    node.enter();
    node_b.enter();
    CHECK( node_b.exit() == ExecutionNode::States::Completed );
    CHECK( node.exit() == ExecutionNode::States::Completed );
    CHECK( nodes.get_root().exit() == ExecutionNode::States::Completed );
}

struct TickTockNode : public CatchKit::Detail::ExecutionNode {
    explicit TickTockNode(CatchKit::Detail::NodeId&& id)
    :   ExecutionNode(std::move(id), 2)
    {}

    int current_value() {
        return current_index + 1;
    }
};

TEST("TickTick Execution Nodes : one node - no children") {
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

    CHECK( node->get_state() == ExecutionNode::States::Entered );

    CHECK( root.exit() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( node->get_state() == ExecutionNode::States::Incomplete,
        "Not explicitly exited node should be exited by its parent");

    root.enter();
    node->enter();

    CHECK( ttnode->current_value() == 2 );

    CHECK( root.exit() == ExecutionNode::States::Completed );
}

TEST( "TickTick Execution Nodes : two nodes" ) {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    auto& root = nodes.get_root();
    root.enter();

    NodeId a_id({"a"}), b_id({"b"});

    nodes.add_node( std::make_unique<TickTockNode>(NodeId(a_id)) );
    auto node = nodes.find_node(a_id);
    REQUIRE( node );
    CHECK( node->get_size() == 2 );

    node->enter();
    CHECK( node->get_state() == ExecutionNode::States::Entered );
    CHECK( node->get_current_index() == 0 );

    nodes.add_node( std::make_unique<TickTockNode>(NodeId(b_id)) );
    auto b_node = nodes.find_node(b_id);
    REQUIRE( b_node );
    CHECK( b_node->get_size() == 2 );
    b_node->enter();
    CHECK( b_node->get_current_index() == 0 );

    CHECK( root.exit() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( node->get_state() == ExecutionNode::States::HasIncompleteChildren);
    CHECK( b_node->get_state() == ExecutionNode::States::Incomplete);

    CHECK( node->get_current_index() == 0 );
    CHECK( b_node->get_current_index() == 1 );

    root.enter();
    node->enter();
    b_node->enter();

    CHECK( root.exit() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( node->get_state() == ExecutionNode::States::Incomplete );
    CHECK( b_node->get_state() == ExecutionNode::States::None );

    CHECK( node->get_current_index() == 1 );
    CHECK( b_node->get_current_index() == 0 );

    root.enter();
    node->enter();
    b_node->enter();

    CHECK( root.exit() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( node->get_state() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( b_node->get_state() == ExecutionNode::States::Incomplete );

    CHECK( node->get_current_index() == 1 );
    CHECK( b_node->get_current_index() == 1 );

    root.enter();
    node->enter();
    b_node->enter();

    CHECK( root.exit() == ExecutionNode::States::Completed );
    CHECK( node->get_state() == ExecutionNode::States::None );
    CHECK( b_node->get_state() == ExecutionNode::States::None );

    CHECK( node->get_current_index() == 0 );
    CHECK( b_node->get_current_index() == 0 );
}
TEST( "One TickTock node, one regular node" ) {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    auto& root = nodes.get_root();
    root.enter();

    NodeId a_id({"a"}), s_id({"b"});

    nodes.add_node( std::make_unique<TickTockNode>(NodeId(a_id)) );
    auto node = nodes.find_node(a_id);

    node->enter();
    CHECK( node->get_state() == ExecutionNode::States::Entered );
    CHECK( node->get_current_index() == 0 );

    nodes.add_node(NodeId(s_id));
    auto s_node = nodes.find_node(s_id);
    CHECK( s_node->get_size() == 1 );
    s_node->enter();
    CHECK( s_node->get_current_index() == 0 );

    CHECK( s_node->exit() == ExecutionNode::States::Completed );

    CHECK( root.exit() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( node->get_state() == ExecutionNode::States::Incomplete);
    CHECK( s_node->get_state() == ExecutionNode::States::None,
        "Node has been reset");

    CHECK( node->get_current_index() == 1 );
    CHECK( s_node->get_current_index() == 0 );

    root.enter();
    node->enter();
    s_node->enter();

    CHECK( s_node->exit() == ExecutionNode::States::Completed );

    CHECK( root.exit() == ExecutionNode::States::Completed );
    CHECK( node->get_state() == ExecutionNode::States::None);
    CHECK( s_node->get_state() == ExecutionNode::States::None);

    CHECK( node->get_current_index() == 0 );
    CHECK( s_node->get_current_index() == 0 );
}

TEST( "One regular node with a nested TickTock node" ) {
    using namespace CatchKit::Detail;

    ExecutionNodes nodes({"root"});
    auto& root = nodes.get_root();
    root.enter();

    NodeId tt_id({"tt"}), s_id({"s"});

    // First go into the "section" node
    nodes.add_node(NodeId(s_id));
    auto s_node = nodes.find_node(s_id);
    CHECK( s_node->get_size() == 1 );
    s_node->enter();
    CHECK( s_node->get_current_index() == 0 );

    // Now, while the section is open, enter the tick tock node
    nodes.add_node( std::make_unique<TickTockNode>(NodeId(tt_id)) );
    auto tt_node = nodes.find_node(tt_id);
    tt_node->enter();
    CHECK( tt_node->get_state() == ExecutionNode::States::Entered );
    CHECK( tt_node->get_current_index() == 0 );

    CHECK( s_node->exit() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( tt_node->get_state() == ExecutionNode::States::Incomplete);

    CHECK( root.exit() == ExecutionNode::States::HasIncompleteChildren );
    CHECK( tt_node->get_state() == ExecutionNode::States::Incomplete );
    CHECK( s_node->get_state() == ExecutionNode::States::HasIncompleteChildren );

    CHECK( tt_node->get_current_index() == 1 );
    CHECK( s_node->get_current_index() == 0 );

    root.enter();
    s_node->enter();
    tt_node->enter();

    CHECK( s_node->exit() == ExecutionNode::States::Completed );
    CHECK( tt_node->get_state() == ExecutionNode::States::None);

    CHECK( root.exit() == ExecutionNode::States::Completed );

    CHECK( tt_node->get_current_index() == 0 );
    CHECK( s_node->get_current_index() == 0 );
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

    CHECK( nodes.get_root().exit() == ExecutionNode::States::HasIncompleteChildren );
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

    CHECK( nodes.get_root().exit() == ExecutionNode::States::HasIncompleteChildren );
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

    CHECK( nodes.get_root().exit() == ExecutionNode::States::HasIncompleteChildren );
    nodes.get_root().enter(); // re-enter whole test

    CHECK( !try_enter_section(nodes, "s1", stable_loc), "should now skip first node");

    CHECK( try_enter_section(nodes, "s2", stable_loc ),
        "should now enter second node at top level");
}
