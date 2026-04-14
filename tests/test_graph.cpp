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

// --- Dijkstra shortest path tests ---

TEST(Dijkstra, SimpleThreeNodePath) {
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 2.0);
    auto result = g.dijkstra("A", "C");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 3.0, 0.001);
    ASSERT_EQ(path.size(), 3);
    ASSERT_EQ(path[0], "A");
    ASSERT_EQ(path[1], "B");
    ASSERT_EQ(path[2], "C");
}

TEST(Dijkstra, ChoosesShorterPath) {
    // A->B->D = 3.0, A->C->D = 7.0 — should pick A->B->D
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("A", "C", 5.0);
    g.add_edge("B", "D", 2.0);
    g.add_edge("C", "D", 2.0);
    auto result = g.dijkstra("A", "D");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 3.0, 0.001);
    ASSERT_EQ(path[0], "A");
    ASSERT_EQ(path[1], "B");
    ASSERT_EQ(path[2], "D");
}

TEST(Dijkstra, FiveNodeNetwork) {
    //   A -1-> B -1-> E
    //   |             ^
    //   2             1
    //   v             |
    //   C ----3----> D
    // Shortest A->E: A->B->E = 2.0
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("A", "C", 2.0);
    g.add_edge("B", "E", 1.0);
    g.add_edge("C", "D", 3.0);
    g.add_edge("D", "E", 1.0);
    auto result = g.dijkstra("A", "E");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 2.0, 0.001);
    ASSERT_EQ(path.size(), 3);
    ASSERT_EQ(path[0], "A");
    ASSERT_EQ(path[1], "B");
    ASSERT_EQ(path[2], "E");
}

TEST(Dijkstra, DirectEdgeShorterThanMultiHop) {
    // Direct A->C = 10, but A->B->C = 3
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 2.0);
    g.add_edge("A", "C", 10.0);
    auto result = g.dijkstra("A", "C");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 3.0, 0.001);
    ASSERT_EQ(path.size(), 3);
}

TEST(Dijkstra, DisconnectedNodes) {
    Graph g;
    g.add_node("A");
    g.add_node("B");
    g.add_edge("A", "C", 1.0);
    auto result = g.dijkstra("A", "B");
    ASSERT_FALSE(result.has_value());
}

TEST(Dijkstra, NonexistentStartNode) {
    Graph g;
    g.add_node("A");
    auto result = g.dijkstra("Z", "A");
    ASSERT_FALSE(result.has_value());
}

TEST(Dijkstra, NonexistentEndNode) {
    Graph g;
    g.add_node("A");
    auto result = g.dijkstra("A", "Z");
    ASSERT_FALSE(result.has_value());
}

TEST(Dijkstra, PathToSelf) {
    Graph g;
    g.add_node("A");
    auto result = g.dijkstra("A", "A");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 0.0, 0.001);
    ASSERT_EQ(path.size(), 1);
    ASSERT_EQ(path[0], "A");
}

TEST(Dijkstra, EqualWeightEdges) {
    // All edges weight 1 — shortest path by hop count
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 1.0);
    g.add_edge("A", "C", 1.0);
    auto result = g.dijkstra("A", "C");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 1.0, 0.001);
    ASSERT_EQ(path.size(), 2);
}

TEST(Dijkstra, LargerGrid) {
    // 4x1 chain: A->B->C->D->E, total = 4.0
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 1.0);
    g.add_edge("C", "D", 1.0);
    g.add_edge("D", "E", 1.0);
    auto result = g.dijkstra("A", "E");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 4.0, 0.001);
    ASSERT_EQ(path.size(), 5);
}

TEST(Dijkstra, UndirectedGraph) {
    Graph g;
    g.add_edge_undirected("A", "B", 2.0);
    g.add_edge_undirected("B", "C", 3.0);
    // Forward
    auto fwd = g.dijkstra("A", "C");
    ASSERT_TRUE(fwd.has_value());
    ASSERT_NEAR(fwd.value().second, 5.0, 0.001);
    // Reverse
    auto rev = g.dijkstra("C", "A");
    ASSERT_TRUE(rev.has_value());
    ASSERT_NEAR(rev.value().second, 5.0, 0.001);
}

// --- A* search tests ---

TEST(AStar, SimplePathWithPositions) {
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    g.add_node("B", std::make_pair(1.0, 0.0));
    g.add_node("C", std::make_pair(2.0, 0.0));
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 1.0);
    auto result = g.a_star("A", "C");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 2.0, 0.001);
    ASSERT_EQ(path.size(), 3);
    ASSERT_EQ(path[0], "A");
    ASSERT_EQ(path[1], "B");
    ASSERT_EQ(path[2], "C");
}

TEST(AStar, ChoosesShorterWeightedPath) {
    // A(0,0) -> B(1,0) -> D(2,0) = 3.0
    // A(0,0) -> C(0,1) -> D(2,0) = 7.0
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    g.add_node("B", std::make_pair(1.0, 0.0));
    g.add_node("C", std::make_pair(0.0, 1.0));
    g.add_node("D", std::make_pair(2.0, 0.0));
    g.add_edge("A", "B", 1.0);
    g.add_edge("A", "C", 5.0);
    g.add_edge("B", "D", 2.0);
    g.add_edge("C", "D", 2.0);
    auto result = g.a_star("A", "D");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 3.0, 0.001);
    ASSERT_EQ(path[1], "B");
}

TEST(AStar, FallbackToDijkstraWithoutPositions) {
    // No positions set — A* should fall back to Dijkstra
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 2.0);
    auto result = g.a_star("A", "C");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 3.0, 0.001);
    ASSERT_EQ(path.size(), 3);
}

TEST(AStar, DisconnectedNodes) {
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    g.add_node("B", std::make_pair(5.0, 5.0));
    auto result = g.a_star("A", "B");
    ASSERT_FALSE(result.has_value());
}

TEST(AStar, NonexistentNode) {
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    auto result = g.a_star("A", "Z");
    ASSERT_FALSE(result.has_value());
}

TEST(AStar, PathToSelf) {
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    auto result = g.a_star("A", "A");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 0.0, 0.001);
    ASSERT_EQ(path.size(), 1);
}

TEST(AStar, GridLikeGraph) {
    //  A(0,0) --1-- B(1,0)
    //  |             |
    //  1             1
    //  |             |
    //  C(0,1) --1-- D(1,1)
    // A->D shortest: A->B->D or A->C->D, both cost 2
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    g.add_node("B", std::make_pair(1.0, 0.0));
    g.add_node("C", std::make_pair(0.0, 1.0));
    g.add_node("D", std::make_pair(1.0, 1.0));
    g.add_edge("A", "B", 1.0);
    g.add_edge("A", "C", 1.0);
    g.add_edge("B", "D", 1.0);
    g.add_edge("C", "D", 1.0);
    auto result = g.a_star("A", "D");
    ASSERT_TRUE(result.has_value());
    auto [path, dist] = result.value();
    ASSERT_NEAR(dist, 2.0, 0.001);
    ASSERT_EQ(path.size(), 3);
}

// --- A* vs Dijkstra comparison tests ---

TEST(AStarVsDijkstra, SameCostOnSimpleGraph) {
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    g.add_node("B", std::make_pair(1.0, 0.0));
    g.add_node("C", std::make_pair(2.0, 0.0));
    g.add_node("D", std::make_pair(3.0, 0.0));
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 1.0);
    g.add_edge("C", "D", 1.0);
    g.add_edge("A", "D", 5.0);
    auto dijk = g.dijkstra("A", "D");
    auto astar = g.a_star("A", "D");
    ASSERT_TRUE(dijk.has_value());
    ASSERT_TRUE(astar.has_value());
    ASSERT_NEAR(dijk.value().second, astar.value().second, 0.001);
}

TEST(AStarVsDijkstra, SameCostOnComplexGraph) {
    Graph g;
    g.add_node("S", std::make_pair(0.0, 0.0));
    g.add_node("A", std::make_pair(1.0, 1.0));
    g.add_node("B", std::make_pair(2.0, 0.0));
    g.add_node("C", std::make_pair(1.0, -1.0));
    g.add_node("D", std::make_pair(3.0, 1.0));
    g.add_node("E", std::make_pair(4.0, 0.0));
    g.add_edge("S", "A", 1.5);
    g.add_edge("S", "B", 2.0);
    g.add_edge("S", "C", 1.8);
    g.add_edge("A", "D", 2.0);
    g.add_edge("B", "D", 1.5);
    g.add_edge("B", "E", 3.0);
    g.add_edge("C", "B", 0.5);
    g.add_edge("D", "E", 1.0);
    auto dijk = g.dijkstra("S", "E");
    auto astar = g.a_star("S", "E");
    ASSERT_TRUE(dijk.has_value());
    ASSERT_TRUE(astar.has_value());
    ASSERT_NEAR(dijk.value().second, astar.value().second, 0.001);
}

TEST(AStarVsDijkstra, BothReturnNulloptWhenDisconnected) {
    Graph g;
    g.add_node("A", std::make_pair(0.0, 0.0));
    g.add_node("B", std::make_pair(10.0, 10.0));
    auto dijk = g.dijkstra("A", "B");
    auto astar = g.a_star("A", "B");
    ASSERT_FALSE(dijk.has_value());
    ASSERT_FALSE(astar.has_value());
}

TEST(Dijkstra, WeightUpdateAffectsPath) {
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("B", "C", 1.0);
    g.add_edge("A", "C", 10.0);
    // Before: A->B->C = 2.0 is shortest
    auto before = g.dijkstra("A", "C");
    ASSERT_NEAR(before.value().second, 2.0, 0.001);
    // Simulate congestion on B->C
    g.update_weight("B", "C", 20.0);
    // After: A->C = 10.0 is now shortest
    auto after = g.dijkstra("A", "C");
    ASSERT_NEAR(after.value().second, 10.0, 0.001);
    ASSERT_EQ(after.value().first.size(), 2);
}
