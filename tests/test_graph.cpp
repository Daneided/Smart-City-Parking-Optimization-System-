// Graph tests — node/edge operations and basic structure

#include "test_framework.h"
#include "../src/data_structures/graph.h"

TEST(Graph, AddNodeAndCheckExists) {
    Graph g;
    g.add_node("A");
    ASSERT_TRUE(g.has_node("A"));
    ASSERT_FALSE(g.has_node("B"));
}

TEST(Graph, AddMultipleNodes) {
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    g.add_node("B", std::make_pair(1.0, 1.0));
    g.add_node("C", std::make_pair(2.0, 2.0));
    ASSERT_TRUE(g.has_node("A"));
    ASSERT_TRUE(g.has_node("B"));
    ASSERT_TRUE(g.has_node("C"));
    ASSERT_EQ(g.nodes().size(), 3);
}

TEST(Graph, AddEdgeCreatesNodes) {
    Graph g;
    g.add_edge("A", "B", 5.0);
    ASSERT_TRUE(g.has_node("A"));
    ASSERT_TRUE(g.has_node("B"));
    ASSERT_TRUE(g.has_edge("A", "B"));
    ASSERT_FALSE(g.has_edge("B", "A"));
}

TEST(Graph, AddUndirectedEdge) {
    Graph g;
    g.add_edge_undirected("A", "B", 3.0);
    ASSERT_TRUE(g.has_edge("A", "B"));
    ASSERT_TRUE(g.has_edge("B", "A"));
}

TEST(Graph, GetWeight) {
    Graph g;
    g.add_edge("A", "B", 4.5);
    auto w = g.get_weight("A", "B");
    ASSERT_TRUE(w.has_value());
    ASSERT_NEAR(w.value(), 4.5, 0.001);
}

TEST(Graph, GetWeightNonexistentEdge) {
    Graph g;
    g.add_node("A");
    auto w = g.get_weight("A", "B");
    ASSERT_FALSE(w.has_value());
}

TEST(Graph, GetNeighbors) {
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("A", "C", 2.0);
    g.add_edge("A", "D", 3.0);
    auto neighbors = g.get_neighbors("A");
    ASSERT_EQ(neighbors.size(), 3);
}

TEST(Graph, GetNeighborsEmpty) {
    Graph g;
    g.add_node("A");
    auto neighbors = g.get_neighbors("A");
    ASSERT_EQ(neighbors.size(), 0);
}

TEST(Graph, RemoveEdge) {
    Graph g;
    g.add_edge("A", "B", 1.0);
    ASSERT_TRUE(g.has_edge("A", "B"));
    g.remove_edge("A", "B");
    ASSERT_FALSE(g.has_edge("A", "B"));
}

TEST(Graph, RemoveNode) {
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 2.0);
    g.add_edge("C", "A", 3.0);
    g.remove_node("B");
    ASSERT_FALSE(g.has_node("B"));
    ASSERT_FALSE(g.has_edge("A", "B"));
    ASSERT_FALSE(g.has_edge("B", "C"));
}

TEST(Graph, UpdateWeight) {
    Graph g;
    g.add_edge("A", "B", 1.0);
    bool updated = g.update_weight("A", "B", 5.0);
    ASSERT_TRUE(updated);
    ASSERT_NEAR(g.get_weight("A", "B").value(), 5.0, 0.001);
}

TEST(Graph, UpdateWeightNonexistentEdge) {
    Graph g;
    g.add_node("A");
    bool updated = g.update_weight("A", "B", 5.0);
    ASSERT_FALSE(updated);
}

TEST(Graph, NodePosition) {
    Graph g;
    g.add_node("A", std::make_pair(3.0, 4.0));
    auto pos = g.get_position("A");
    ASSERT_TRUE(pos.has_value());
    ASSERT_NEAR(pos.value().first, 3.0, 0.001);
    ASSERT_NEAR(pos.value().second, 4.0, 0.001);
}

TEST(Graph, NodePositionNotSet) {
    Graph g;
    g.add_node("A");
    auto pos = g.get_position("A");
    ASSERT_FALSE(pos.has_value());
}
