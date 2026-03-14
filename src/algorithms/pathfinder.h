// Abstract pathfinder interface — demonstrates OOP inheritance hierarchy
// Concrete implementations: DijkstraPathfinder, AStarPathfinder

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../data_structures/graph.h"

// Abstract base class for pathfinding algorithms.
// Enables runtime strategy selection (Dijkstra vs A*) via polymorphism.
class IPathfinder {
public:
    virtual ~IPathfinder() = default;

    // Find shortest path from start to end.
    // Returns (path, total_distance) or nullopt if no path exists.
    virtual std::optional<std::pair<std::vector<std::string>, double>>
    find_path(const std::string& start, const std::string& end) = 0;

    // Human-readable algorithm name for logging/benchmarking.
    virtual std::string algorithm_name() const = 0;
};

// Concrete pathfinder using Dijkstra's algorithm.
// Guarantees optimal shortest path; explores all directions uniformly.
class DijkstraPathfinder : public IPathfinder {
public:
    DijkstraPathfinder(Graph* graph) : _graph(graph) {}

    std::optional<std::pair<std::vector<std::string>, double>>
    find_path(const std::string& start, const std::string& end) override {
        if (_graph == nullptr) return std::nullopt;
        return _graph->dijkstra(start, end);
    }

    std::string algorithm_name() const override {
        return "Dijkstra";
    }

private:
    Graph* _graph;
};
