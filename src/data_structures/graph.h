// Weighted graph for parking area navigation
// Used by RouteOptimizer for A*/Dijkstra pathfinding

#ifndef GRAPH_H
#define GRAPH_H

#include <cmath>
#include <limits>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Weighted directed graph using adjacency list representation.
// Nodes represent locations (entrances, intersections, spots, exits).
// Edges represent drivable paths with distance/time weights.
// Supports dynamic weight updates for congestion modeling.
class Graph {
public:
    Graph() = default;

    std::unordered_set<std::string> nodes() const {
        std::unordered_set<std::string> result;
        for (const auto& [id, _] : _adjacency) {
            result.insert(id);
        }
        return result;
    }

    // Add a node. Position is (x, y) used as A* heuristic.
    void add_node(const std::string& node_id,
                  std::optional<std::pair<double, double>> position = std::nullopt) {
        if (_adjacency.find(node_id) == _adjacency.end()) {
            _adjacency[node_id] = {};
        }
        if (position.has_value()) {
            _positions[node_id] = position.value();
        }
    }

    void add_edge(const std::string& from_id, const std::string& to_id, double weight) {
        if (_adjacency.find(from_id) == _adjacency.end()) {
            _adjacency[from_id] = {};
        }
        if (_adjacency.find(to_id) == _adjacency.end()) {
            _adjacency[to_id] = {};
        }
        _adjacency[from_id][to_id] = weight;
    }

    void add_edge_undirected(const std::string& node_a, const std::string& node_b, double weight) {
        add_edge(node_a, node_b, weight);
        add_edge(node_b, node_a, weight);
    }

    void remove_edge(const std::string& from_id, const std::string& to_id) {
        auto it = _adjacency.find(from_id);
        if (it != _adjacency.end()) {
            it->second.erase(to_id);
        }
    }

    void remove_node(const std::string& node_id) {
        _adjacency.erase(node_id);
        _positions.erase(node_id);
        for (auto& [_, neighbors] : _adjacency) {
            neighbors.erase(node_id);
        }
    }

    std::vector<std::pair<std::string, double>> get_neighbors(const std::string& node_id) const {
        auto it = _adjacency.find(node_id);
        if (it == _adjacency.end()) return {};
        std::vector<std::pair<std::string, double>> result;
        for (const auto& [neighbor, weight] : it->second) {
            result.emplace_back(neighbor, weight);
        }
        return result;
    }

    std::optional<double> get_weight(const std::string& from_id, const std::string& to_id) const {
        auto it = _adjacency.find(from_id);
        if (it == _adjacency.end()) return std::nullopt;
        auto edge_it = it->second.find(to_id);
        if (edge_it == it->second.end()) return std::nullopt;
        return edge_it->second;
    }

    // Update edge weight (e.g. for congestion). Returns false if edge doesn't exist.
    bool update_weight(const std::string& from_id, const std::string& to_id, double weight) {
        auto it = _adjacency.find(from_id);
        if (it == _adjacency.end()) return false;
        auto edge_it = it->second.find(to_id);
        if (edge_it == it->second.end()) return false;
        edge_it->second = weight;
        return true;
    }

    std::optional<std::pair<double, double>> get_position(const std::string& node_id) const {
        auto it = _positions.find(node_id);
        if (it == _positions.end()) return std::nullopt;
        return it->second;
    }

    bool has_node(const std::string& node_id) const {
        return _adjacency.find(node_id) != _adjacency.end();
    }

    bool has_edge(const std::string& from_id, const std::string& to_id) const {
        auto it = _adjacency.find(from_id);
        if (it == _adjacency.end()) return false;
        return it->second.find(to_id) != it->second.end();
    }

    // Shortest path using Dijkstra's algorithm.
    std::optional<std::pair<std::vector<std::string>, double>>
    dijkstra(const std::string& start, const std::string& end) const { return std::nullopt; }

    // A* search using euclidean distance heuristic. Falls back to Dijkstra if positions not set.
    std::optional<std::pair<std::vector<std::string>, double>>
    a_star(const std::string& start, const std::string& end) const { return std::nullopt; }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, double>> _adjacency;
    std::unordered_map<std::string, std::pair<double, double>> _positions;

    double _heuristic(const std::string& node, std::pair<double, double> target_pos) const {
        return 0.0;
    }

    std::vector<std::string> _reconstruct_path(
        const std::unordered_map<std::string, std::string>& prev,
        const std::string& end) const {
        return {};
    }
};

#endif // GRAPH_H
