#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <cmath>
#include <limits>

#include "../data_structures/graph.h"

struct Route {
    std::vector<std::string> nodes;
    double total_distance;
    double estimated_time;
};

struct AllocationResult {
    std::unordered_map<std::string, std::string> assignments;
    double total_cost;
    std::vector<std::string> unassigned;
};

class RouteOptimizer {
public:
    static constexpr double PARKING_SPEED = 5.0;

    RouteOptimizer(Graph* parking_graph = nullptr)
        : _graph(parking_graph) {}

    void register_exit(const std::string& exit_id) {
        _exits.insert(exit_id);
    }

    std::optional<Route> find_route_to_spot(const std::string& entrance_id,
                                            const std::string& spot_id);
    std::optional<Route> find_route_to_exit(const std::string& spot_id,
                                            std::optional<std::string> preferred_exit = std::nullopt);
    void update_congestion(const std::string& from_id, const std::string& to_id,
                           double delay);

private:
    Graph* _graph;
    std::unordered_set<std::string> _exits;
};
