#include <iostream>
#include <string>
#include <unordered_map>

#include "data_structures/quadtree.h"
#include "data_structures/graph.h"
#include "data_structures/priority_queue.h"
#include "models/parking_spot.h"
#include "models/availability_tracker.h"
#include "algorithms/search.h"
#include "algorithms/pathfinder.h"
#include "algorithms/optimization.h"

int main() {
    std::cout << "=== Smart City Parking Optimization System ===" << std::endl;

    // --- Models: parking spots + availability tracking ---
    ParkingSpot spot_a("S1", {1.0, 2.0}, "zone-A");
    ParkingSpot spot_b("S2", {3.0, 4.0}, "zone-A", "handicap");
    ParkingSpot spot_c("S3", {8.0, 9.0}, "zone-B");

    AvailabilityTracker tracker;
    spot_a.register_status_callback(
        [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            tracker.on_status_change(id, old_s, new_s);
        });
    spot_b.register_status_callback(
        [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            tracker.on_status_change(id, old_s, new_s);
        });
    spot_c.register_status_callback(
        [&tracker](const std::string& id, SpotStatus old_s, SpotStatus new_s) {
            tracker.on_status_change(id, old_s, new_s);
        });

    tracker.register_spot("S1", "zone-A", true);
    tracker.register_spot("S2", "zone-A", true);
    tracker.register_spot("S3", "zone-B", true);

    std::cout << "\nAvailable spots: " << tracker.count_available() << std::endl;
    spot_b.occupy();
    std::cout << "After occupying S2: " << tracker.count_available() << " available" << std::endl;
    std::cout << "Zone-A occupancy rate: " << tracker.get_zone_occupancy_rate("zone-A") << std::endl;

    // --- Spatial index: QuadTree ---
    BoundingBox bounds(0.0, 0.0, 10.0, 10.0);
    QuadTree tree(bounds);

    std::unordered_map<std::string, std::string> data_s1 = {
        {"spot_id", "S1"}, {"zone_id", "zone-A"}, {"spot_type", "standard"}};
    std::unordered_map<std::string, std::string> data_s2 = {
        {"spot_id", "S2"}, {"zone_id", "zone-A"}, {"spot_type", "handicap"}};
    std::unordered_map<std::string, std::string> data_s3 = {
        {"spot_id", "S3"}, {"zone_id", "zone-B"}, {"spot_type", "standard"}};

    tree.insert(Point(1.0, 2.0, data_s1));
    tree.insert(Point(3.0, 4.0, data_s2));
    tree.insert(Point(8.0, 9.0, data_s3));

    // --- Search: SpotSearcher ---
    SpotSearcher searcher(&tree, [&tracker](const std::string& id) {
        return tracker.is_available(id);
    });

    SearchCriteria criteria;
    criteria.location = {0.0, 0.0};
    criteria.max_results = 5;

    auto results = searcher.search(criteria);
    std::cout << "\nSearch results (from origin):" << std::endl;
    for (const auto& r : results) {
        std::cout << "  " << r.spot_id << " dist=" << r.distance
                  << " zone=" << r.zone_id << std::endl;
    }

    auto nearest = searcher.search_nearest({0.0, 0.0});
    if (nearest.has_value()) {
        std::cout << "Nearest available: " << nearest->spot_id << std::endl;
    }

    // --- Graph: pathfinding ---
    Graph graph;
    graph.add_node("entrance", {{0.0, 0.0}});
    graph.add_node("intersect1", {{2.0, 2.0}});
    graph.add_node("intersect2", {{5.0, 5.0}});
    graph.add_node("S1", {{1.0, 2.0}});
    graph.add_node("S3", {{8.0, 9.0}});
    graph.add_node("exit", {{10.0, 0.0}});

    graph.add_edge_undirected("entrance", "intersect1", 2.83);
    graph.add_edge_undirected("intersect1", "S1", 1.0);
    graph.add_edge_undirected("intersect1", "intersect2", 4.24);
    graph.add_edge_undirected("intersect2", "S3", 5.0);
    graph.add_edge_undirected("intersect2", "exit", 7.07);

    // --- Optimization: RouteOptimizer with polymorphic pathfinder ---
    AStarPathfinder astar_pf(&graph);
    DijkstraPathfinder dijkstra_pf(&graph);

    RouteOptimizer router(&astar_pf, &graph);
    router.register_exit("exit");

    std::cout << "\nUsing pathfinder: " << router.pathfinder_name() << std::endl;

    auto route = router.find_route_to_spot("entrance", "S1");
    if (route.has_value()) {
        std::cout << "Route entrance -> S1: ";
        for (size_t i = 0; i < route->nodes.size(); ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << route->nodes[i];
        }
        std::cout << " (dist=" << route->total_distance
                  << ", time=" << route->estimated_time << "s)" << std::endl;
    }

    auto exit_route = router.find_route_to_exit("S3");
    if (exit_route.has_value()) {
        std::cout << "Route S3 -> exit: ";
        for (size_t i = 0; i < exit_route->nodes.size(); ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << exit_route->nodes[i];
        }
        std::cout << " (dist=" << exit_route->total_distance << ")" << std::endl;
    }

    // Swap to Dijkstra via polymorphism
    RouteOptimizer router_dijkstra(&dijkstra_pf, &graph);
    router_dijkstra.register_exit("exit");
    std::cout << "\nUsing pathfinder: " << router_dijkstra.pathfinder_name() << std::endl;

    auto route_d = router_dijkstra.find_route_to_spot("entrance", "S1");
    if (route_d.has_value()) {
        std::cout << "Route entrance -> S1: ";
        for (size_t i = 0; i < route_d->nodes.size(); ++i) {
            if (i > 0) std::cout << " -> ";
            std::cout << route_d->nodes[i];
        }
        std::cout << " (dist=" << route_d->total_distance
                  << ", time=" << route_d->estimated_time << "s)" << std::endl;
    }

    // --- Optimization: AllocationOptimizer ---
    AllocationOptimizer allocator;
    std::vector<std::pair<std::string, std::pair<double, double>>> requests = {
        {"req1", {0.0, 0.0}},
        {"req2", {7.0, 8.0}},
    };
    std::vector<std::pair<std::string, std::pair<double, double>>> spots = {
        {"S1", {1.0, 2.0}},
        {"S3", {8.0, 9.0}},
    };

    auto alloc = allocator.allocate_greedy(requests, spots);
    std::cout << "\nGreedy allocation:" << std::endl;
    for (const auto& [req, spot] : alloc.assignments) {
        std::cout << "  " << req << " -> " << spot << std::endl;
    }
    std::cout << "Total cost: " << alloc.total_cost << std::endl;

    std::cout << "\n=== All systems operational ===" << std::endl;
    return 0;
}
