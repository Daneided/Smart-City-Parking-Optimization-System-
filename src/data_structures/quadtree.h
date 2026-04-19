// QuadTree for spatial indexing of parking spots
// Supports range queries and k-nearest neighbor search

#ifndef QUADTREE_H
#define QUADTREE_H

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "../models/common_types.h"

// A 2D point with associated data (e.g. spot_id)
struct Point {
    double x;
    double y;
    SpotData data;

    Point(double x, double y, SpotData data = {})
        : x(x), y(y), data(std::move(data)) {}

    double distance_to(double other_x, double other_y) const {
        double dx = x - other_x;
        double dy = y - other_y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

// Axis-aligned bounding box defined by center and half-dimensions
struct BoundingBox {
    double cx;
    double cy;
    double half_w;
    double half_h;

    BoundingBox(double cx, double cy, double half_w, double half_h)
        : cx(cx), cy(cy), half_w(half_w), half_h(half_h) {}

    bool contains(double x, double y) const {
        return std::abs(x - cx) <= half_w &&
               std::abs(y - cy) <= half_h;
    }

    bool intersects(const BoundingBox& other) const {
        return !(other.cx - other.half_w > cx + half_w ||
                 other.cx + other.half_w < cx - half_w ||
                 other.cy - other.half_h > cy + half_h ||
                 other.cy + other.half_h < cy - half_h);
    }

    // Minimum distance from a point to any point in this box
    double min_distance_to(double x, double y) const {
        double dx = std::max(0.0, std::abs(x - cx) - half_w);
        double dy = std::max(0.0, std::abs(y - cy) - half_h);
        return std::sqrt(dx * dx + dy * dy);
    }

    std::tuple<BoundingBox, BoundingBox, BoundingBox, BoundingBox> subdivide() const {
        double qw = half_w / 2.0;
        double qh = half_h / 2.0;
        return {
            BoundingBox(cx - qw, cy - qh, qw, qh),  // SW
            BoundingBox(cx + qw, cy - qh, qw, qh),  // SE
            BoundingBox(cx - qw, cy + qh, qw, qh),  // NW
            BoundingBox(cx + qw, cy + qh, qw, qh),  // NE
        };
    }
};

// QuadTree for 2D spatial indexing.
// Stores points and supports insert/remove, range query, and k-nearest neighbor search.
// Capacity: max points per leaf before subdivision.
class QuadTree {
public:
    QuadTree(const BoundingBox& boundary, int capacity = 4)
        : boundary(boundary), capacity(capacity), divided(false), _size(0) {}

    int size() const { return _size; }

    // Insert a point. Returns false if point is outside boundary.
    bool insert(const Point& point) {
        if (!boundary.contains(point.x, point.y)) {
            return false;
        }

        if (!divided && static_cast<int>(points.size()) < capacity) {
            points.push_back(point);
            _size++;
            return true;
        }

        if (!divided) {
            _subdivide();
        }

        if (nw->insert(point)) { _size++; return true; }
        if (ne->insert(point)) { _size++; return true; }
        if (sw->insert(point)) { _size++; return true; }
        if (se->insert(point)) { _size++; return true; }

        return false;
    }

    // Remove a point by coordinates. Returns true if a point was removed.
    bool remove(double x, double y) {
        if (!boundary.contains(x, y)) {
            return false;
        }

        for (auto it = points.begin(); it != points.end(); ++it) {
            if (it->x == x && it->y == y) {
                points.erase(it);
                _size--;
                return true;
            }
        }

        if (divided) {
            QuadTree* children[] = {nw.get(), ne.get(), sw.get(), se.get()};
            for (auto* child : children) {
                if (child->remove(x, y)) {
                    _size--;
                    _try_merge();
                    return true;
                }
            }
        }

        return false;
    }

    // Find all points within the given bounding box.
    std::vector<Point> query_range(const BoundingBox& search_box) {
        std::vector<Point> found;
        _query_range(search_box, found);
        return found;
    }

    // Find all points within radius of (x, y). Returns (point, distance) pairs.
    std::vector<std::pair<Point, double>> query_radius(double x, double y, double radius) {
        // Use bounding box for coarse filter, then check exact distance
        BoundingBox search_box(x, y, radius, radius);
        std::vector<Point> candidates = query_range(search_box);
        std::vector<std::pair<Point, double>> results;
        double r_sq = radius * radius;
        for (const auto& point : candidates) {
            double dx = point.x - x;
            double dy = point.y - y;
            double dist_sq = dx * dx + dy * dy;
            if (dist_sq <= r_sq) {
                results.emplace_back(point, std::sqrt(dist_sq));
            }
        }
        return results;
    }

    // Find k nearest points using branch-and-bound with max-heap pruning.
    // Returns list of (point, distance) sorted by distance ascending.
    std::vector<std::pair<Point, double>> k_nearest(double x, double y, int k = 1) {
        // best is maintained as a max-heap during search (largest distance at front)
        std::vector<std::pair<double, Point>> best;
        _k_nearest(x, y, k, best);
        // Sort ascending by distance for final output
        std::sort(best.begin(), best.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        std::vector<std::pair<Point, double>> result;
        result.reserve(best.size());
        for (const auto& [d, p] : best) {
            result.emplace_back(p, d);
        }
        return result;
    }

    BoundingBox boundary;
    int capacity;
    std::vector<Point> points;
    bool divided;
    std::unique_ptr<QuadTree> nw;
    std::unique_ptr<QuadTree> ne;
    std::unique_ptr<QuadTree> sw;
    std::unique_ptr<QuadTree> se;

private:
    int _size;

    void _subdivide() {
        auto [sw_b, se_b, nw_b, ne_b] = boundary.subdivide();
        sw = std::make_unique<QuadTree>(sw_b, capacity);
        se = std::make_unique<QuadTree>(se_b, capacity);
        nw = std::make_unique<QuadTree>(nw_b, capacity);
        ne = std::make_unique<QuadTree>(ne_b, capacity);
        divided = true;

        // Re-insert existing points into children
        std::vector<Point> remaining = std::move(points);
        points.clear();
        for (const auto& point : remaining) {
            bool inserted = nw->insert(point) || ne->insert(point) ||
                            sw->insert(point) || se->insert(point);
            if (!inserted) {
                // Edge case: point sits exactly on boundary, keep in parent
                points.push_back(point);
            }
        }
    }
    // Collapse children back into parent if total points fit in capacity.
    void _try_merge() {
        if (!divided) {
            return;
        }

        int total = static_cast<int>(nw->points.size() + ne->points.size() +
                                     sw->points.size() + se->points.size());

        // Only merge if none of the children are subdivided
        if (total <= capacity &&
                !nw->divided && !ne->divided &&
                !sw->divided && !se->divided) {
            points.insert(points.end(), nw->points.begin(), nw->points.end());
            points.insert(points.end(), ne->points.begin(), ne->points.end());
            points.insert(points.end(), sw->points.begin(), sw->points.end());
            points.insert(points.end(), se->points.begin(), se->points.end());
            nw.reset();
            ne.reset();
            sw.reset();
            se.reset();
            divided = false;
        }
    }
    void _query_range(const BoundingBox& search_box, std::vector<Point>& found) {
        if (!boundary.intersects(search_box)) {
            return;
        }

        for (const auto& point : points) {
            if (search_box.contains(point.x, point.y)) {
                found.push_back(point);
            }
        }

        if (divided) {
            nw->_query_range(search_box, found);
            ne->_query_range(search_box, found);
            sw->_query_range(search_box, found);
            se->_query_range(search_box, found);
        }
    }
    // Max-heap comparator: largest distance at front for efficient pruning
    static bool _heap_cmp(const std::pair<double, Point>& a, const std::pair<double, Point>& b) {
        return a.first < b.first;
    }

    // Branch-and-bound k-NN using max-heap. O(n log k) instead of O(n * k log k).
    void _k_nearest(double x, double y, int k, std::vector<std::pair<double, Point>>& best) {
        double min_dist = boundary.min_distance_to(x, y);
        if (static_cast<int>(best.size()) >= k && min_dist > best.front().first) {
            return;  // prune: this subtree can't beat current k-th best
        }

        for (const auto& point : points) {
            double dist = point.distance_to(x, y);
            if (static_cast<int>(best.size()) < k) {
                best.emplace_back(dist, point);
                std::push_heap(best.begin(), best.end(), _heap_cmp);
            } else if (dist < best.front().first) {
                std::pop_heap(best.begin(), best.end(), _heap_cmp);
                best.back() = {dist, point};
                std::push_heap(best.begin(), best.end(), _heap_cmp);
            }
        }

        if (!divided) {
            return;
        }

        // Visit children in order of proximity to query point for better pruning
        std::vector<std::pair<double, QuadTree*>> children = {
            {nw->boundary.min_distance_to(x, y), nw.get()},
            {ne->boundary.min_distance_to(x, y), ne.get()},
            {sw->boundary.min_distance_to(x, y), sw.get()},
            {se->boundary.min_distance_to(x, y), se.get()},
        };
        std::sort(children.begin(), children.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        for (const auto& [_, child] : children) {
            child->_k_nearest(x, y, k, best);
        }
    }
};

#endif // QUADTREE_H
