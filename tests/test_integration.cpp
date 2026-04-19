// Integration tests — full pipeline: search -> route -> allocate

#include "test_framework.h"
#include "../src/data_structures/quadtree.h"
#include "../src/data_structures/graph.h"
#include "../src/models/parking_spot.h"
#include "../src/models/availability_tracker.h"
#include "../src/algorithms/search.h"
#include "../src/algorithms/pathfinder.h"
#include "../src/algorithms/optimization.h"

static SpotData spot_data(const std::string& id, const std::string& zone) {
    return SpotData{id, zone, "standard"};
}

TEST(Integration, SearchThenRoute) {
    // Set up spots and tracker
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", false); // occupied
    tracker.register_spot("S3", "zone-B", true);

    // Spatial index
    QuadTree tree(BoundingBox(5.0, 5.0, 5.0, 5.0));
    tree.insert(Point(2.0, 2.0, spot_data("S1", "zone-A")));
    tree.insert(Point(3.0, 3.0, spot_data("S2", "zone-A")));
    tree.insert(Point(8.0, 8.0, spot_data("S3", "zone-B")));

    // Search
    SpotSearcher searcher(&tree, [&tracker](const std::string& id) {
        return tracker.is_available(id);
    });
    auto nearest = searcher.search_nearest({0.0, 0.0});
    ASSERT_TRUE(nearest.has_value());
    ASSERT_EQ(nearest->spot_id, "S1"); // S2 is occupied, S1 is closest available

    // Build graph and route to found spot
    Graph graph;
    graph.add_node("entrance", std::make_pair(0.0, 0.0));
    graph.add_node("S1", std::make_pair(2.0, 2.0));
    graph.add_edge_undirected("entrance", "S1", 2.83);

    AStarPathfinder pathfinder(&graph);
    RouteOptimizer router(&pathfinder, &graph);
    auto route = router.find_route_to_spot("entrance", nearest->spot_id);
    ASSERT_TRUE(route.has_value());
    ASSERT_NEAR(route->total_distance, 2.83, 0.01);
    ASSERT_EQ(route->nodes.front(), "entrance");
    ASSERT_EQ(route->nodes.back(), "S1");
}

TEST(Integration, AllocateThenRoute) {
    // Allocate spots to drivers
    AllocationOptimizer allocator;
    std::vector<std::pair<std::string, std::pair<double, double>>> requests = {
        {"driver1", {0.0, 0.0}},
        {"driver2", {10.0, 10.0}},
    };
    std::vector<std::pair<std::string, std::pair<double, double>>> spots = {
        {"S1", {1.0, 1.0}},
        {"S2", {9.0, 9.0}},
    };
    auto alloc = allocator.allocate_greedy(requests, spots);
    ASSERT_EQ(alloc.assignments.at("driver1"), "S1");
    ASSERT_EQ(alloc.assignments.at("driver2"), "S2");

    // Route driver1 to assigned spot
    Graph graph;
    graph.add_node("entrance", std::make_pair(0.0, 0.0));
    graph.add_node("S1", std::make_pair(1.0, 1.0));
    graph.add_edge_undirected("entrance", "S1", 1.41);

    DijkstraPathfinder pathfinder(&graph);
    RouteOptimizer router(&pathfinder, &graph);
    auto route = router.find_route_to_spot("entrance", alloc.assignments.at("driver1"));
    ASSERT_TRUE(route.has_value());
    ASSERT_NEAR(route->total_distance, 1.41, 0.01);
}

TEST(Integration, CallbackTriggersResearch) {
    // Set up two spots, search finds S1
    AvailabilityTracker tracker;
    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", true);

    QuadTree tree(BoundingBox(5.0, 5.0, 5.0, 5.0));
    tree.insert(Point(1.0, 1.0, spot_data("S1", "zone-A")));
    tree.insert(Point(8.0, 8.0, spot_data("S2", "zone-A")));

    SpotSearcher searcher(&tree, [&tracker](const std::string& id) {
        return tracker.is_available(id);
    });

    auto first = searcher.search_nearest({0.0, 0.0});
    ASSERT_EQ(first->spot_id, "S1");

    // S1 gets taken via callback
    ParkingSpot spot_s1("S1", {1.0, 1.0}, "zone-A");
    spot_s1.register_status_callback(
        [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            tracker.on_status_change(id, old_s, new_s);
        });
    spot_s1.occupy();
    ASSERT_FALSE(tracker.is_available("S1"));

    // Re-search now finds S2
    auto second = searcher.search_nearest({0.0, 0.0});
    ASSERT_TRUE(second.has_value());
    ASSERT_EQ(second->spot_id, "S2");
}

TEST(Integration, CongestionReroute) {
    Graph graph;
    graph.add_node("entrance", std::make_pair(0.0, 0.0));
    graph.add_node("A", std::make_pair(5.0, 5.0));
    graph.add_node("B", std::make_pair(5.0, -5.0));
    graph.add_node("spot", std::make_pair(10.0, 0.0));

    // North route: entrance->A->spot = 2.0 + 2.0 = 4.0
    graph.add_edge("entrance", "A", 2.0);
    graph.add_edge("A", "spot", 2.0);
    // South route: entrance->B->spot = 2.5 + 2.5 = 5.0
    graph.add_edge("entrance", "B", 2.5);
    graph.add_edge("B", "spot", 2.5);

    AStarPathfinder pathfinder(&graph);
    RouteOptimizer router(&pathfinder, &graph);

    // Before congestion: north route is shorter
    auto before = router.find_route_to_spot("entrance", "spot");
    ASSERT_TRUE(before.has_value());
    ASSERT_NEAR(before->total_distance, 4.0, 0.01);
    ASSERT_EQ(before->nodes[1], "A");

    // Add congestion to north route
    router.update_congestion("entrance", "A", 10.0);

    // After: south route wins (5.0 < 12.0)
    auto after = router.find_route_to_spot("entrance", "spot");
    ASSERT_TRUE(after.has_value());
    ASSERT_NEAR(after->total_distance, 5.0, 0.01);
    ASSERT_EQ(after->nodes[1], "B");
}

TEST(Integration, FullPipeline) {
    // Complete flow: create spots, track availability, search, route, allocate, exit
    AvailabilityTracker tracker;
    std::vector<ParkingSpot> spots;
    spots.emplace_back("S1", std::make_pair(5.0, 5.0), "zone-A");
    spots.emplace_back("S2", std::make_pair(15.0, 5.0), "zone-A");

    for (auto& sp : spots) {
        tracker.register_spot(sp.spot_id, sp.zone_id, true);
        sp.register_status_callback(
            [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
                tracker.on_status_change(id, old_s, new_s);
            });
    }

    QuadTree tree(BoundingBox(10.0, 5.0, 10.0, 5.0));
    tree.insert(Point(5.0, 5.0, spot_data("S1", "zone-A")));
    tree.insert(Point(15.0, 5.0, spot_data("S2", "zone-A")));

    Graph graph;
    graph.add_node("entrance", std::make_pair(0.0, 5.0));
    graph.add_node("S1", std::make_pair(5.0, 5.0));
    graph.add_node("S2", std::make_pair(15.0, 5.0));
    graph.add_node("exit", std::make_pair(20.0, 5.0));
    graph.add_edge_undirected("entrance", "S1", 5.0);
    graph.add_edge_undirected("S1", "S2", 10.0);
    graph.add_edge_undirected("S2", "exit", 5.0);

    // Search
    SpotSearcher searcher(&tree, [&tracker](const std::string& id) {
        return tracker.is_available(id);
    });
    auto found = searcher.search_nearest({0.0, 5.0});
    ASSERT_EQ(found->spot_id, "S1");

    // Route
    AStarPathfinder pf(&graph);
    RouteOptimizer router(&pf, &graph);
    router.register_exit("exit");
    auto route = router.find_route_to_spot("entrance", "S1");
    ASSERT_TRUE(route.has_value());
    ASSERT_NEAR(route->total_distance, 5.0, 0.01);

    // Park (occupy via callback)
    spots[0].occupy();
    ASSERT_FALSE(tracker.is_available("S1"));
    ASSERT_NEAR(tracker.get_zone_occupancy_rate("zone-A"), 0.5, 0.001);

    // Exit route
    auto exit_route = router.find_route_to_exit("S1");
    ASSERT_TRUE(exit_route.has_value());
}