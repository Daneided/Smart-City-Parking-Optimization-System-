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
    // Returns (path, total_distance) or nullopt if no path exists.
    std::optional<std::pair<std::vector<std::string>, double>>
    dijkstra(const std::string& start, const std::string& end) const {
        if (_adjacency.find(start) == _adjacency.end() ||
            _adjacency.find(end) == _adjacency.end()) {
            return std::nullopt;
        }

        std::unordered_map<std::string, double> dist;
        std::unordered_map<std::string, std::string> prev;
        std::unordered_set<std::string> visited;
        dist[start] = 0.0;

        // Min-heap: (distance, node_id)
        using Entry = std::pair<double, std::string>;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> heap;
        heap.push({0.0, start});

        while (!heap.empty()) {
            auto [d, node] = heap.top();
            heap.pop();

            if (visited.count(node)) continue;
            visited.insert(node);

            if (node == end) {
                return std::pair{_reconstruct_path(prev, end), d};
            }

            auto adj_it = _adjacency.find(node);
            if (adj_it == _adjacency.end()) continue;

            for (const auto& [neighbor, weight] : adj_it->second) {
                if (visited.count(neighbor)) continue;
                double new_dist = d + weight;
                if (dist.find(neighbor) == dist.end() || new_dist < dist[neighbor]) {
                    dist[neighbor] = new_dist;
                    prev[neighbor] = node;
                    heap.push({new_dist, neighbor});
                }
            }
        }

        return std::nullopt;
    }

    // A* search using euclidean distance heuristic. Falls back to Dijkstra if positions not set.
    std::optional<std::pair<std::vector<std::string>, double>>
    a_star(const std::string& start, const std::string& end) const {
        if (_adjacency.find(start) == _adjacency.end() ||
            _adjacency.find(end) == _adjacency.end()) {
            return std::nullopt;
        }

        auto end_pos_it = _positions.find(end);
        if (end_pos_it == _positions.end()) {
            return dijkstra(start, end);
        }
        auto end_pos = end_pos_it->second;

        std::unordered_map<std::string, double> g_score;
        std::unordered_map<std::string, std::string> prev;
        std::unordered_set<std::string> visited;
        g_score[start] = 0.0;

        // Min-heap: (f_score, node_id)
        using Entry = std::pair<double, std::string>;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> heap;
        heap.push({_heuristic(start, end_pos), start});

        while (!heap.empty()) {
            auto [_, node] = heap.top();
            heap.pop();

            if (visited.count(node)) continue;
            visited.insert(node);

            if (node == end) {
                return std::pair{_reconstruct_path(prev, end), g_score[end]};
            }

            auto adj_it = _adjacency.find(node);
            if (adj_it == _adjacency.end()) continue;

            for (const auto& [neighbor, weight] : adj_it->second) {
                if (visited.count(neighbor)) continue;
                double tentative_g = g_score[node] + weight;
                if (g_score.find(neighbor) == g_score.end() || tentative_g < g_score[neighbor]) {
                    g_score[neighbor] = tentative_g;
                    prev[neighbor] = node;
                    double f = tentative_g + _heuristic(neighbor, end_pos);
                    heap.push({f, neighbor});
                }
            }
        }

        return std::nullopt;
    }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, double>> _adjacency;
    std::unordered_map<std::string, std::pair<double, double>> _positions;

    double _heuristic(const std::string& node, std::pair<double, double> target_pos) const {
        auto it = _positions.find(node);
        if (it == _positions.end()) return 0.0;
        double dx = it->second.first - target_pos.first;
        double dy = it->second.second - target_pos.second;
        return std::sqrt(dx * dx + dy * dy);
    }

    std::vector<std::string> _reconstruct_path(
        const std::unordered_map<std::string, std::string>& prev,
        const std::string& end) const {
        std::vector<std::string> path;
        std::string node = end;
        while (true) {
            path.push_back(node);
            auto it = prev.find(node);
            if (it == prev.end()) break;
            node = it->second;
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

#endif // GRAPH_H
