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
#include "../models/common_types.h"
#include "pathfinder.h"

struct AllocEntry {
    std::string id;
    Coordinate loc;
};

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

    RouteOptimizer(IPathfinder* pathfinder = nullptr, Graph* graph = nullptr)
        : _pathfinder(pathfinder), _graph(graph) {}

    void register_exit(const std::string& exit_id) {
        _exits.insert(exit_id);
    }

    std::optional<Route> find_route_to_spot(const std::string& entrance_id,
                                            const std::string& spot_id) {
        if (_pathfinder == nullptr) return std::nullopt;
        auto result = _pathfinder->find_path(entrance_id, spot_id);
        if (!result.has_value()) return std::nullopt;
        auto& [path, distance] = result.value();
        return Route{path, distance, distance / PARKING_SPEED};
    }

    std::optional<Route> find_route_to_exit(const std::string& spot_id,
                                            std::optional<std::string> preferred_exit = std::nullopt) {
        if (_pathfinder == nullptr) return std::nullopt;

        if (preferred_exit.has_value()) {
            auto result = _pathfinder->find_path(spot_id, preferred_exit.value());
            if (!result.has_value()) return std::nullopt;
            auto& [path, distance] = result.value();
            return Route{path, distance, distance / PARKING_SPEED};
        }

        std::optional<Route> best = std::nullopt;
        for (const auto& exit_id : _exits) {
            auto result = _pathfinder->find_path(spot_id, exit_id);
            if (!result.has_value()) continue;
            auto& [path, distance] = result.value();
            if (!best.has_value() || distance < best->total_distance) {
                best = Route{path, distance, distance / PARKING_SPEED};
            }
        }
        return best;
    }

    void update_congestion(const std::string& from_id, const std::string& to_id,
                           double delay) {
        if (_graph == nullptr) return;
        auto current = _graph->get_weight(from_id, to_id);
        if (current.has_value()) {
            _graph->update_weight(from_id, to_id, current.value() + delay);
        }
    }

    std::string pathfinder_name() const {
        if (_pathfinder == nullptr) return "none";
        return _pathfinder->algorithm_name();
    }

private:
    IPathfinder* _pathfinder;
    Graph* _graph;
    std::unordered_set<std::string> _exits;
};

class AllocationOptimizer {
public:
    AllocationOptimizer() = default;

    AllocationResult allocate_greedy(
            const std::vector<AllocEntry>& requests,
            const std::vector<AllocEntry>& available_spots) {
        std::unordered_map<std::string, std::string> assignments;
        std::vector<std::string> unassigned;
        double total_cost = 0.0;

        auto remaining = available_spots;

        for (const auto& req : requests) {
            if (remaining.empty()) {
                unassigned.push_back(req.id);
                continue;
            }

            size_t best_idx = 0;
            double best_dist = std::numeric_limits<double>::infinity();

            for (size_t i = 0; i < remaining.size(); ++i) {
                double dist = req.loc.distance_to(remaining[i].loc);
                if (dist < best_dist) {
                    best_dist = dist;
                    best_idx = i;
                }
            }

            assignments[req.id] = remaining[best_idx].id;
            total_cost += best_dist;
            remaining.erase(remaining.begin() + static_cast<long>(best_idx));
        }

        return AllocationResult{assignments, total_cost, unassigned};
    }
};
