// AllocationOptimizer and RouteOptimizer tests

#include "test_framework.h"
#include "../src/algorithms/optimization.h"

// --- AllocationOptimizer tests ---

TEST(Allocation, SimpleAssignment) {
    AllocationOptimizer alloc;
    std::vector<AllocEntry> requests = {
        {"driver1", {0.0, 0.0}},
        {"driver2", {10.0, 10.0}}
    };
    std::vector<AllocEntry> spots = {
        {"S1", {1.0, 1.0}},
        {"S2", {9.0, 9.0}},
        {"S3", {50.0, 50.0}}
    };

    auto result = alloc.allocate_greedy(requests, spots);
    ASSERT_EQ(result.assignments.size(), 2);
    ASSERT_EQ(result.unassigned.size(), 0);
    // driver1 should get S1 (closest), driver2 should get S2
    ASSERT_EQ(result.assignments.at("driver1"), "S1");
    ASSERT_EQ(result.assignments.at("driver2"), "S2");
}

TEST(Allocation, MoreDriversThanSpots) {
    AllocationOptimizer alloc;
    std::vector<AllocEntry> requests = {
        {"driver1", {0.0, 0.0}},
        {"driver2", {10.0, 10.0}},
        {"driver3", {20.0, 20.0}}
    };
    std::vector<AllocEntry> spots = {
        {"S1", {1.0, 1.0}}
    };

    auto result = alloc.allocate_greedy(requests, spots);
    ASSERT_EQ(result.assignments.size(), 1);
    ASSERT_EQ(result.unassigned.size(), 2);
    ASSERT_EQ(result.assignments.at("driver1"), "S1");
}

TEST(Allocation, NoSpots) {
    AllocationOptimizer alloc;
    std::vector<AllocEntry> requests = {
        {"driver1", {0.0, 0.0}}
    };
    std::vector<AllocEntry> spots;

    auto result = alloc.allocate_greedy(requests, spots);
    ASSERT_EQ(result.assignments.size(), 0);
    ASSERT_EQ(result.unassigned.size(), 1);
    ASSERT_EQ(result.unassigned[0], "driver1");
}

TEST(Allocation, NoDrivers) {
    AllocationOptimizer alloc;
    std::vector<AllocEntry> requests;
    std::vector<AllocEntry> spots = {
        {"S1", {1.0, 1.0}}
    };

    auto result = alloc.allocate_greedy(requests, spots);
    ASSERT_EQ(result.assignments.size(), 0);
    ASSERT_EQ(result.unassigned.size(), 0);
    ASSERT_NEAR(result.total_cost, 0.0, 0.001);
}

TEST(Allocation, TotalCostAccumulates) {
    AllocationOptimizer alloc;
    std::vector<AllocEntry> requests = {
        {"driver1", {0.0, 0.0}},
        {"driver2", {10.0, 0.0}}
    };
    std::vector<AllocEntry> spots = {
        {"S1", {3.0, 4.0}},   // dist from driver1 = 5.0
        {"S2", {10.0, 0.0}}   // dist from driver2 = 0.0
    };

    auto result = alloc.allocate_greedy(requests, spots);
    ASSERT_NEAR(result.total_cost, 5.0, 0.001);
}

TEST(Allocation, GreedyAssignsClosestEach) {
    AllocationOptimizer alloc;
    // Both drivers near S1, but first driver gets it, second gets S2
    std::vector<AllocEntry> requests = {
        {"driver1", {0.0, 0.0}},
        {"driver2", {1.0, 1.0}}
    };
    std::vector<AllocEntry> spots = {
        {"S1", {0.5, 0.5}},
        {"S2", {50.0, 50.0}}
    };

    auto result = alloc.allocate_greedy(requests, spots);
    // driver1 processed first, gets closest S1
    ASSERT_EQ(result.assignments.at("driver1"), "S1");
    // driver2 gets remaining S2
    ASSERT_EQ(result.assignments.at("driver2"), "S2");
}

TEST(Allocation, SingleDriverSingleSpot) {
    AllocationOptimizer alloc;
    std::vector<AllocEntry> requests = {
        {"driver1", {3.0, 4.0}}
    };
    std::vector<AllocEntry> spots = {
        {"S1", {0.0, 0.0}}
    };

    auto result = alloc.allocate_greedy(requests, spots);
    ASSERT_EQ(result.assignments.size(), 1);
    ASSERT_EQ(result.assignments.at("driver1"), "S1");
    ASSERT_NEAR(result.total_cost, 5.0, 0.001);
}

// --- RouteOptimizer tests ---

TEST(RouteOptimizer, FindRouteToSpot) {
    Graph g;
    g.add_node("entrance", std::make_pair(0.0, 0.0));
    g.add_node("mid", std::make_pair(1.0, 0.0));
    g.add_node("spot1", std::make_pair(2.0, 0.0));
    g.add_edge("entrance", "mid", 1.0);
    g.add_edge("mid", "spot1", 1.0);

    AStarPathfinder pathfinder(&g);
    RouteOptimizer optimizer(&pathfinder, &g);

    auto route = optimizer.find_route_to_spot("entrance", "spot1");
    ASSERT_TRUE(route.has_value());
    ASSERT_NEAR(route->total_distance, 2.0, 0.001);
    ASSERT_EQ(route->nodes.size(), 3);
    ASSERT_EQ(route->nodes[0], "entrance");
    ASSERT_EQ(route->nodes[2], "spot1");
}

TEST(RouteOptimizer, FindRouteToSpotNoPath) {
    Graph g;
    g.add_node("entrance");
    g.add_node("spot1");

    DijkstraPathfinder pathfinder(&g);
    RouteOptimizer optimizer(&pathfinder, &g);

    auto route = optimizer.find_route_to_spot("entrance", "spot1");
    ASSERT_FALSE(route.has_value());
}

TEST(RouteOptimizer, FindRouteToExitPreferred) {
    Graph g;
    g.add_edge("spot1", "exitA", 5.0);
    g.add_edge("spot1", "exitB", 3.0);

    DijkstraPathfinder pathfinder(&g);
    RouteOptimizer optimizer(&pathfinder, &g);
    optimizer.register_exit("exitA");
    optimizer.register_exit("exitB");

    // With preferred exit
    auto route = optimizer.find_route_to_exit("spot1", "exitA");
    ASSERT_TRUE(route.has_value());
    ASSERT_NEAR(route->total_distance, 5.0, 0.001);
}

TEST(RouteOptimizer, FindRouteToNearestExit) {
    Graph g;
    g.add_edge("spot1", "exitA", 5.0);
    g.add_edge("spot1", "exitB", 3.0);

    DijkstraPathfinder pathfinder(&g);
    RouteOptimizer optimizer(&pathfinder, &g);
    optimizer.register_exit("exitA");
    optimizer.register_exit("exitB");

    // Without preferred exit — should pick nearest
    auto route = optimizer.find_route_to_exit("spot1");
    ASSERT_TRUE(route.has_value());
    ASSERT_NEAR(route->total_distance, 3.0, 0.001);
}

TEST(RouteOptimizer, FindRouteToExitNoExitsRegistered) {
    Graph g;
    g.add_edge("spot1", "exitA", 5.0);

    DijkstraPathfinder pathfinder(&g);
    RouteOptimizer optimizer(&pathfinder, &g);

    auto route = optimizer.find_route_to_exit("spot1");
    ASSERT_FALSE(route.has_value());
}

TEST(RouteOptimizer, NullPathfinder) {
    RouteOptimizer optimizer(nullptr, nullptr);
    auto route = optimizer.find_route_to_spot("A", "B");
    ASSERT_FALSE(route.has_value());
}

TEST(RouteOptimizer, EstimatedTime) {
    Graph g;
    g.add_edge("A", "B", 10.0);

    DijkstraPathfinder pathfinder(&g);
    RouteOptimizer optimizer(&pathfinder, &g);

    auto route = optimizer.find_route_to_spot("A", "B");
    ASSERT_TRUE(route.has_value());
    // time = distance / PARKING_SPEED (5.0)
    ASSERT_NEAR(route->estimated_time, 2.0, 0.001);
}

TEST(RouteOptimizer, CongestionUpdate) {
    Graph g;
    g.add_edge("A", "B", 1.0);
    g.add_edge("A", "C", 10.0);
    g.add_edge("B", "C", 1.0);

    DijkstraPathfinder pathfinder(&g);
    RouteOptimizer optimizer(&pathfinder, &g);

    // Before congestion: A->B->C = 2.0
    auto before = optimizer.find_route_to_spot("A", "C");
    ASSERT_NEAR(before->total_distance, 2.0, 0.001);

    // Add heavy congestion to A->B
    optimizer.update_congestion("A", "B", 20.0);

    // After congestion: A->B = 21.0, so A->C direct (10.0) is now shorter
    auto after = optimizer.find_route_to_spot("A", "C");
    ASSERT_NEAR(after->total_distance, 10.0, 0.001);
}

TEST(RouteOptimizer, PathfinderName) {
    Graph g;
    DijkstraPathfinder dijk(&g);
    AStarPathfinder astar(&g);

    RouteOptimizer opt_dijk(&dijk, &g);
    RouteOptimizer opt_astar(&astar, &g);
    RouteOptimizer opt_none(nullptr, nullptr);

    ASSERT_EQ(opt_dijk.pathfinder_name(), "Dijkstra");
    ASSERT_EQ(opt_astar.pathfinder_name(), "A*");
    ASSERT_EQ(opt_none.pathfinder_name(), "none");
}
