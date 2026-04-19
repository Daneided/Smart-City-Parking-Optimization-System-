#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../data_structures/graph.h"

// Pathfinder strategy interface: returns (path, distance) or nullopt.
class IPathfinder {
public:
    virtual ~IPathfinder() = default;

    virtual std::optional<std::pair<std::vector<std::string>, double>>
    find_path(const std::string& start, const std::string& end) = 0;

    virtual std::string algorithm_name() const = 0;
};

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

class AStarPathfinder : public IPathfinder {
public:
    AStarPathfinder(Graph* graph) : _graph(graph) {}

    std::optional<std::pair<std::vector<std::string>, double>>
    find_path(const std::string& start, const std::string& end) override {
        if (_graph == nullptr) return std::nullopt;
        return _graph->a_star(start, end);
    }

    std::string algorithm_name() const override {
        return "A*";
    }

private:
    Graph* _graph;
};
